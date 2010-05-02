//-------------------------------------------------------------------------------------------------
// <copyright file="Expressions.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
//      The expression praser.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "Operands.h"

namespace IronMan
{

    struct Expression
    {
        virtual ~Expression() {}
        virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand) = 0;
        virtual Expression* Clone() = 0;
    };

    // Arithmetic Expressions:  LessThan, LessThanOrEqualTo, Equals, GreaterThanOrEqualTo, GreaterThan
    class ArithmeticExpression : public Expression
    {
        ILogger& m_logger;
        CString  m_lhs;
        CString  m_expressionName;
        bool     m_whenNonExistent;
        Operand* m_pOperand;
    protected:
        ArithmeticExpression(const ArithmeticExpression& rhs)
            : m_logger(rhs.m_logger)
            , m_lhs(rhs.m_lhs)
            , m_expressionName(rhs.m_expressionName)
            , m_whenNonExistent(rhs.m_whenNonExistent)
            , m_pOperand(rhs.m_pOperand->Clone())
        {}

    public:
        ArithmeticExpression(CComPtr<IXMLDOMElement>& spElement, const CString& elementName, ILogger& logger)
            : m_logger(logger)
            , m_lhs(ElementUtils::GetAttributeByName(spElement, L"LeftHandSide", logger, true))
            , m_whenNonExistent(StringToBool(ElementUtils::GetAttributeByName(spElement, L"BoolWhenNonExistent", logger, true), logger))
            , m_pOperand(OperandUtils::GetOperandFromChild(spElement, logger))
            , m_expressionName(elementName)
        {
            ElementUtils::VerifyName(spElement, elementName, logger);
            if (1 != ElementUtils::CountChildElements(spElement))
            {
                CInvalidXmlException ixe(L"schema validation failure: " + elementName + L" must have exactly 1 child node");
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }

            // Only some Operators support Inequality comparisons(LessThan, GreaterThan, ...)
            if ( "Equals" != elementName && !m_pOperand->SupportsLessThanGreaterThan())
            {
                CInvalidXmlException ixe(elementName + L" has an unsupported operand: " + m_pOperand->OperandName() + L".");
                // exception is logged farther up the line
                throw ixe;
            }
        }
        virtual ~ArithmeticExpression() { delete m_pOperand; }

        virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand)
        {
            CString section(L" evaluated to false"); // the default
            PUSHLOGSECTIONPOP(m_logger, m_expressionName, L"evaluating", section);

            CString value;            
            bool b = m_pOperand->Evaluate(ipdtoDataToOperand, value);
            if (b == false)
            {
                if (m_whenNonExistent)
                {
                    section = L" evaluated to true";
                    LOG(m_logger, ILogger::Information, L"returning BoolWhenNonExistent's value: true");
                }
                else
                    LOG(m_logger, ILogger::Information, L"returning BoolWhenNonExistent's value: false");
                return m_whenNonExistent;
            }

            CString left, right;
            if (MSIUtils::IsAllNumeric(m_lhs) && MSIUtils::IsAllNumeric(value))
            {
                CString tlhs(m_lhs);
                CString tvalue(value);

                // special leading and trailing . handling
                if (tlhs[0]   == L'.')
                    tlhs = L"0" + tlhs;
                if (tvalue[0] == L'.')
                    tvalue = L"0" + tvalue;
                if (!tlhs.IsEmpty()  && tlhs  [  tlhs.GetLength()-1] == L'.')
                    tlhs += L"0";
                if (!tvalue.IsEmpty() && tvalue[tvalue.GetLength()-1] == L'.')
                    tvalue += L"0";

                // equalize the number of dots, by appending
                while (MSIUtils::DotCount(tlhs) < MSIUtils::DotCount(tvalue))
                    tlhs += L".0";
                while (MSIUtils::DotCount(tvalue) < MSIUtils::DotCount(tlhs))
                    tvalue += L".0";

                int leftStart=0, rightStart=0;

                for(;;)
                {
                    CString leftPart  = tlhs.Tokenize(L".", leftStart);
                    CString rightPart = tvalue.Tokenize(L".", rightStart);

                    if ((leftStart == -1) || (rightStart == -1))
                    {
                        if ((leftStart != -1) || (rightStart != -1))
                        {	// I've tried and I've tried to get the number of dots to match,
                            // but just in case (for instance, two dots in a row),
                            // handle anything unrecognizable lexically
                            left  = L"." + m_lhs;
                            right = L"." + value;
                            LOG(m_logger, ILogger::Verbose, L"unrecognizable numeric - not canonicalizing");
                        }
                        else
                        {
                            LOG(m_logger, ILogger::Verbose, L"all numeric characters - canonicalizing");
                        }
                        break;
                    }

                    // pad segment with leading 0s
                    while(leftPart.GetLength() < rightPart.GetLength())
                        leftPart = L"0" + leftPart;
                    while(rightPart.GetLength() < leftPart.GetLength())
                        rightPart = L"0" + rightPart;

                    left  += L"." + leftPart;
                    right += L"." + rightPart;
                }
                left  = left.Mid(1); // strip off leading .s
                right = right.Mid(1);
            }
            else
            {
                left = m_lhs;
                right = value;
            }

            b = EvaluateTemplateMethod(left, right);
            if (b)
                section = L" evaluated to true";
            return b;
        }

    private: // "template method"
        virtual bool EvaluateTemplateMethod(const CString& left, const CString& right) = 0;

    private:
        static bool StringToBool(const CString& cs, ILogger& logger)
        {
            if (cs.CompareNoCase(L"false") == 0)	return false;
            if (cs.CompareNoCase(L"true") == 0)		return true;

            CInvalidXmlException ixe(L"schema validation error:  bad value for bool: " + cs);
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
    };
    class LessThan : public ArithmeticExpression
    {
        virtual bool EvaluateTemplateMethod(const CString& left, const CString& right) {return left < right; }
    public:
        LessThan(CComPtr<IXMLDOMElement>& spElement, ILogger& logger) : ArithmeticExpression(spElement, L"LessThan", logger) {}
        LessThan* Clone() { return new LessThan(*this); }
    };
    class LessThanOrEqualTo : public ArithmeticExpression
    {
        virtual bool EvaluateTemplateMethod(const CString& left, const CString& right) {return left <= right; }
    public:
        LessThanOrEqualTo(CComPtr<IXMLDOMElement>& spElement, ILogger& logger) : ArithmeticExpression(spElement, L"LessThanOrEqualTo", logger) {}
        LessThanOrEqualTo* Clone() { return new LessThanOrEqualTo(*this); }
    };
    class Equals : public ArithmeticExpression
    {
        virtual bool EvaluateTemplateMethod(const CString& left, const CString& right) {return left == right; }
    public:
        Equals(CComPtr<IXMLDOMElement>& spElement, ILogger& logger) : ArithmeticExpression(spElement, L"Equals", logger) {}
        Equals* Clone() { return new Equals(*this); }
    };
    class GreaterThanOrEqualTo : public ArithmeticExpression
    {		
        virtual bool EvaluateTemplateMethod(const CString& left, const CString& right) {return left >= right; }
    public:
        GreaterThanOrEqualTo(CComPtr<IXMLDOMElement>& spElement, ILogger& logger) : ArithmeticExpression(spElement, L"GreaterThanOrEqualTo", logger) {}
        GreaterThanOrEqualTo* Clone() { return new GreaterThanOrEqualTo(*this); }
    };
    class GreaterThan : public ArithmeticExpression
    {
        virtual bool EvaluateTemplateMethod(const CString& left, const CString& right) {return left > right; }
    public:
        GreaterThan(CComPtr<IXMLDOMElement>& spElement, ILogger& logger) : ArithmeticExpression(spElement, L"GreaterThan", logger) {}
        GreaterThan* Clone() { return new GreaterThan(*this); }
    };

    // special unary expression
    class Exists : public Expression
    {
        ILogger& m_logger;
        Operand* m_pOperand;
        Exists(const Exists& rhs)
            : m_logger(rhs.m_logger)
            , m_pOperand(rhs.m_pOperand->Clone())
        {}
    public:
        Exists(CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
            : m_logger(logger)
            , m_pOperand(OperandUtils::GetOperandFromChild(spElement, logger))
        {
            ElementUtils::VerifyName(spElement, L"Exists", logger);
            if (1 != ElementUtils::CountChildElements(spElement))
            {
                CInvalidXmlException ixe(L"schema validation failure: Exists must have exactly 1 child node");
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }
        }
        virtual ~Exists() { delete m_pOperand; }

        virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand)
        {
            CString section(L" evaluated to false"); // the default
            PUSHLOGSECTIONPOP(m_logger, L"Exists", L"evaluating", section);

            CString cs;		
            bool b = m_pOperand->Evaluate(ipdtoDataToOperand, cs);
            if (b)
                section = L" evaluated to true";
            return b;
        }
        Exists* Clone() { return new Exists(*this); }
    };

    template<bool b> class Always : public Expression
    {
        ILogger& m_logger;
    public:
        Always(ILogger& logger) : m_logger(logger) {}
        bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand)
        {
            CString message = b ? L"returning true" : L"returning false";
            LOG(m_logger, ILogger::Information, message);
            return b;
        }
        Always<b>* Clone() { return new Always<b>(m_logger); }

    };
    typedef Always<true> AlwaysTrue;
    typedef Always<false> NeverTrue;

    // LogicalExpressions: And, Or, Not
    Expression* MakeExpression(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger); // forward declaration
    class LogicalExpression : public Expression
    {
        ILogger& m_logger;
        CString m_expressionName;
        Expression* m_left;
        Expression* m_right;
    protected:
        LogicalExpression(const LogicalExpression& rhs)
            : m_logger(rhs.m_logger)
            , m_expressionName(rhs.m_expressionName)
            , m_left  (rhs.m_left->Clone())
            , m_right (rhs.m_right->Clone())
        {}
    public:
        LogicalExpression(const CComPtr<IXMLDOMElement>& spElement, const CString& elementName, ILogger& logger)
            : m_logger(logger)
            , m_expressionName(elementName)
            , m_left (MakeExpression(ElementUtils::FindChildElementByNumber(spElement, 0, logger), logger))
            , m_right(MakeExpression(ElementUtils::FindChildElementByNumber(spElement, 1, logger), logger))
        {
            ElementUtils::VerifyName(spElement, elementName, logger);

            if (2 != ElementUtils::CountChildElements(spElement))
            {
                CInvalidXmlException ixe(L"schema validation failure: " + elementName + L" must have exactly 2 child nodes");
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }
        }
        virtual ~LogicalExpression()
        {
            delete m_left;
            delete m_right;
        }
        virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand)
        {
            CString section(L" evaluated to false"); // the default
            PUSHLOGSECTIONPOP(m_logger, m_expressionName, L"evaluating", section);

            // the following two lines are necessary, because otherwise they're evaluated in reverse order (stdcall)
            bool left  = m_left->Evaluate(ipdtoDataToOperand);
            bool right = m_right->Evaluate(ipdtoDataToOperand);
            bool b = EvaluateTemplateMethod(left, right);
            if (b)
                section = L" evaluated to true";
            return b;
        }

    private: // "template method"
        virtual bool EvaluateTemplateMethod(bool left, bool right) = 0;
    };
    class And : public LogicalExpression
    {
        virtual bool EvaluateTemplateMethod(bool left, bool right) { return left && right; }
    public:
        And(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger) : LogicalExpression(spElement, L"And", logger) {}
        And* Clone() { return new And(*this); }
    };
    class Or : public LogicalExpression
    {
        virtual bool EvaluateTemplateMethod(bool left, bool right) { return left || right; }
    public:
        Or(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger) : LogicalExpression(spElement, L"Or", logger) {}
        Or* Clone() { return new Or(*this); }
    };
    class Not : public Expression
    {
        ILogger& m_logger;
        Expression* m_exp;
        Not(const Not& rhs)
            : m_logger(rhs.m_logger)
            , m_exp(rhs.m_exp->Clone())
        {}
    public:
        Not(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
            : m_logger(logger)
            , m_exp (MakeExpression(ElementUtils::FindChildElementByNumber(spElement, 0, logger), logger))
        {
            ElementUtils::VerifyName(spElement, L"Not", logger);
            if (1 != ElementUtils::CountChildElements(spElement))
            {
                CInvalidXmlException ixe(L"schema validation failure: Not must have exactly 1 child node");
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }
        }
        virtual ~Not() { delete m_exp; }

    public: // Expression
        virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand)
        {
            CString section(L" evaluated to false"); // the default
            PUSHLOGSECTIONPOP(m_logger, L"Not", L"evaluating", section);

            bool b = !m_exp->Evaluate(ipdtoDataToOperand);
            if (b)
                section = L" evaluated to true";
            return b;
        }
        Not* Clone() { return new Not(*this); }
    };

    // class factory function
    inline Expression* MakeExpression(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
    {   
        CString name;
        CComPtr<IXMLDOMElement> spElementLocal = XmlUtils::ExpandExpressionAlias(spElement, logger, name);
     
        // logical expressions
        if (name == L"And")	return new And(spElementLocal, logger);
        if (name == L"Or")	return new Or (spElementLocal, logger);
        if (name == L"Not")	return new Not(spElementLocal, logger);

        // arithmetic
        if (name == L"LessThan")				return new LessThan			(spElementLocal, logger);
        if (name == L"LessThanOrEqualTo")		return new LessThanOrEqualTo (spElementLocal, logger);
        if (name == L"Equals")					return new Equals			  (spElementLocal, logger);
        if (name == L"GreaterThanOrEqualTo")	return new GreaterThanOrEqualTo(spElementLocal, logger);
        if (name == L"GreaterThan")				return new GreaterThan			(spElementLocal, logger);

        // special unary expression
        if (name == L"Exists")	return new Exists(spElementLocal, logger);
        if (name == L"AlwaysTrue")	return new AlwaysTrue(logger);
        if (name == L"NeverTrue")	return new NeverTrue(logger);

        CInvalidXmlException ixe(L"schema validation failure: unknown Expression: " + name);
        LOG(logger, ILogger::Error, ixe.GetMessage());
        throw ixe;
    }

    class BaseIf
    {
        Expression * m_pExpression;
        ILogger& m_logger;
        const CString m_derivedName;
    public:
        BaseIf(const CComPtr<IXMLDOMElement>& spElement, const CString& derivedName, ILogger& logger)
            : m_pExpression(MakeExpression(ElementUtils::FindChildElementByNumber(spElement, 0, logger), logger))
            , m_logger(logger)
            , m_derivedName(derivedName)
        {
            if (1 != ElementUtils::CountChildElements(spElement))
            {
                CInvalidXmlException ixe(L"schema validation failure:  " + derivedName + L" can only have one logical or arithmietic expression for a child node");
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }
        }
        BaseIf(bool bWantTrue = true, ILogger& logger = NullLogger::GetNullLogger())
            : m_pExpression(bWantTrue ? static_cast<Expression*>(new AlwaysTrue(logger)) : new NeverTrue(logger))
            , m_logger(logger)
        {}
        BaseIf(const BaseIf& rhs)
            : m_pExpression(rhs.m_pExpression->Clone())
            , m_logger(rhs.m_logger)
            , m_derivedName(rhs.m_derivedName)
        {}
        virtual ~BaseIf() { delete m_pExpression; }

        //Make it virtual so that dervied class can override it.
        //This is used by Patches element to OR up the Evaluate of its MSP elements.
        virtual bool Evaluate(const IProvideDataToOperand& ipdtoDataToOperand) const
        {
            LOG(m_logger, ILogger::Information, L"evaluating " + m_derivedName + L":");
            return m_pExpression->Evaluate(ipdtoDataToOperand);
        }
    };

    struct IsPresent : public BaseIf
    {
        IsPresent() : BaseIf(false) {}
        IsPresent(const IsPresent& rhs) : BaseIf(rhs) {}
        IsPresent(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger) : BaseIf(spElement, L"IsPresent", logger) 
        {
            if (ElementUtils::CountChildElements(ElementUtils::GetParentElement(spElement), L"IsPresent") > 1)
            {
                CInvalidXmlException ixe(L"schema validation failure:  IsPresent can only be authored once.");
                throw ixe;
            }
        }
    };

    struct ApplicableIf : public BaseIf
    {
        ApplicableIf() : BaseIf() {}
        ApplicableIf(const ApplicableIf& rhs) : BaseIf(rhs) {}
        ApplicableIf(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger) : BaseIf(spElement, L"ApplicableIf", logger) 
        {
            if (ElementUtils::CountChildElements(ElementUtils::GetParentElement(spElement), L"ApplicableIf") > 1)
            {
                CInvalidXmlException ixe(L"schema validation failure:  IsPresent can only be authored once.");
                throw ixe;
            }
        }
    };

    struct EnterMaintenanceModeIf : public BaseIf
    {
        EnterMaintenanceModeIf() : BaseIf() {}
        EnterMaintenanceModeIf(const EnterMaintenanceModeIf& rhs) : BaseIf(rhs) {}
        EnterMaintenanceModeIf(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger) : BaseIf(spElement, L"EnterMaintenanceModeIf", logger) {}
    };

    struct BlockIf : public BaseIf
    {
        BlockIf() : BaseIf() {}
        BlockIf(const BlockIf& rhs) : BaseIf(rhs) {}
        BlockIf(const CComPtr<IXMLDOMElement>& spElement, ILogger& logger) : BaseIf(spElement, L"BlockIf", logger) {}
        BlockIf(const CComPtr<IXMLDOMNode>& spNode, ILogger& logger) : BaseIf(CComQIPtr<IXMLDOMElement>(spNode), L"BlockIf", logger) {}
    };
}
