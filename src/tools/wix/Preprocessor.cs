//-------------------------------------------------------------------------------------------------
// <copyright file="Preprocessor.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Preprocessor of the WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace WixToolset
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics.CodeAnalysis;
    using System.Globalization;
    using System.IO;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Xml;
    using System.Xml.Linq;
    using WixToolset.Extensibility;

    /// <summary>
    /// Preprocessor object
    /// </summary>
    public sealed class Preprocessor
    {
        private readonly Regex defineRegex = new Regex(@"^\s*(?<varName>.+?)\s*(=\s*(?<varValue>.+?)\s*)?$", RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.ExplicitCapture);
        private readonly Regex pragmaRegex = new Regex(@"^\s*(?<pragmaName>.+?)(?<pragmaValue>[\s\(].+?)?$", RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.ExplicitCapture);

        private readonly XmlReaderSettings DocumentXmlReaderSettings = new XmlReaderSettings()
        {
            ValidationFlags = System.Xml.Schema.XmlSchemaValidationFlags.None,
            XmlResolver = null,
        };
        private readonly XmlReaderSettings FragmentXmlReaderSettings = new XmlReaderSettings()
        {
            ConformanceLevel = System.Xml.ConformanceLevel.Fragment,
            ValidationFlags = System.Xml.Schema.XmlSchemaValidationFlags.None,
            XmlResolver = null,
        };

        private ArrayList extensions;
        private Hashtable extensionsByPrefix;
        private List<InspectorExtension> inspectorExtensions;

        private SourceLineNumber currentLineNumber;
        private Stack<SourceLineNumber> sourceStack;

        private PreprocessorCore core;
        private TextWriter preprocessOut;

        private Stack<bool> includeNextStack;
        private Stack<string> currentFileStack;

        private Platform currentPlatform;

        /// <summary>
        /// Creates a new preprocesor.
        /// </summary>
        public Preprocessor()
        {
            this.IncludeSearchPaths = new List<string>();

            this.extensions = new ArrayList();
            this.extensionsByPrefix = new Hashtable();
            this.inspectorExtensions = new List<InspectorExtension>();

            this.sourceStack = new Stack<SourceLineNumber>();

            this.includeNextStack = new Stack<bool>();
            this.currentFileStack = new Stack<string>();

            this.currentPlatform = Platform.X86;
        }

        /// <summary>
        /// Event for ifdef/ifndef directives.
        /// </summary>
        public event IfDefEventHandler IfDef;

        /// <summary>
        /// Event for included files.
        /// </summary>
        public event IncludedFileEventHandler IncludedFile;

        /// <summary>
        /// Event for messages.
        /// </summary>
        public event MessageEventHandler Message;

        /// <summary>
        /// Event for preprocessed stream.
        /// </summary>
        public event ProcessedStreamEventHandler ProcessedStream;

        /// <summary>
        /// Event for resolved variables.
        /// </summary>
        public event ResolvedVariableEventHandler ResolvedVariable;

        /// <summary>
        /// Enumeration for preprocessor operations in if statements.
        /// </summary>
        private enum PreprocessorOperation
        {
            /// <summary>The and operator.</summary>
            And,

            /// <summary>The or operator.</summary>
            Or,

            /// <summary>The not operator.</summary>
            Not
        }

        /// <summary>
        /// Gets or sets the platform which the compiler will use when defaulting 64-bit attributes and elements.
        /// </summary>
        /// <value>The platform which the compiler will use when defaulting 64-bit attributes and elements.</value>
        public Platform CurrentPlatform
        {
            get { return this.currentPlatform; }
            set { this.currentPlatform = value; }
        }

        /// <summary>
        /// Ordered list of search paths that the precompiler uses to find included files.
        /// </summary>
        /// <value>List of ordered search paths to use during precompiling.</value>
        public IList<string> IncludeSearchPaths { get; private set; }

        /// <summary>
        /// Specifies the text stream to display the postprocessed data to.
        /// </summary>
        /// <value>TextWriter to write preprocessed xml to.</value>
        public TextWriter PreprocessOut
        {
            get { return this.preprocessOut; }
            set { this.preprocessOut = value; }
        }

        /// <summary>
        /// Get the source line information for the current element.  The precompiler will insert
        /// special source line number processing instructions before each element that it
        /// encounters.  This is where those line numbers are read and processed.  This function
        /// may return an array of source line numbers because the element may have come from
        /// an included file, in which case the chain of imports is expressed in the array.
        /// </summary>
        /// <param name="node">Element to get source line information for.</param>
        /// <returns>Returns the stack of imports used to author the element being processed.</returns>
        [SuppressMessage("Microsoft.Design", "CA1059:MembersShouldNotExposeCertainConcreteTypes")]
        public static SourceLineNumber GetSourceLineNumbers(XmlNode node)
        {
            return null;
        }

        /// <summary>
        /// Get the source line information for the current element.  The precompiler will insert
        /// special source line number information for each element that it encounters.
        /// </summary>
        /// <param name="node">Element to get source line information for.</param>
        /// <returns>
        /// The source line number used to author the element being processed or
        /// null if the preprocessor did not process the element or the node is
        /// not an element.
        /// </returns>
        public static SourceLineNumber GetSourceLineNumbers(XObject node)
        {
            return node.Annotation<SourceLineNumber>();
        }

        /// <summary>
        /// Adds an extension.
        /// </summary>
        /// <param name="extension">The extension to add.</param>
        public void AddExtension(WixExtension extension)
        {
            if (null != extension.PreprocessorExtension)
            {
                this.extensions.Add(extension.PreprocessorExtension);

                if (null != extension.PreprocessorExtension.Prefixes)
                {
                    foreach (string prefix in extension.PreprocessorExtension.Prefixes)
                    {
                        PreprocessorExtension collidingExtension = (PreprocessorExtension)this.extensionsByPrefix[prefix];

                        if (null == collidingExtension)
                        {
                            this.extensionsByPrefix.Add(prefix, extension.PreprocessorExtension);
                        }
                        else
                        {
                            throw new WixException(WixErrors.DuplicateExtensionPreprocessorType(extension.GetType().ToString(), prefix, collidingExtension.GetType().ToString()));
                        }
                    }
                }
            }

            if (null != extension.InspectorExtension)
            {
                this.inspectorExtensions.Add(extension.InspectorExtension);
            }
        }

        /// <summary>
        /// Preprocesses a file.
        /// </summary>
        /// <param name="sourceFile">The file to preprocess.</param>
        /// <param name="variables">The variables defined prior to preprocessing.</param>
        /// <returns>XDocument with the postprocessed data.</returns>
        [SuppressMessage("Microsoft.Design", "CA1059:MembersShouldNotExposeCertainConcreteTypes")]
        public XDocument Process(string sourceFile, IDictionary<string, string> variables)
        {
            using (Stream sourceStream = new FileStream(sourceFile, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                InspectorCore inspectorCore = new InspectorCore(this.Message);
                foreach (InspectorExtension inspectorExtension in this.inspectorExtensions)
                {
                    inspectorExtension.Core = inspectorCore;
                    inspectorExtension.InspectSource(sourceStream);

                    // reset
                    inspectorExtension.Core = null;
                    sourceStream.Position = 0;
                }

                if (inspectorCore.EncounteredError)
                {
                    return null;
                }

                using (XmlReader reader = XmlReader.Create(sourceFile, DocumentXmlReaderSettings))
                {
                    return Process(reader, variables, sourceFile);
                }
            }
        }

        /// <summary>
        /// Preprocesses a file.
        /// </summary>
        /// <param name="sourceFile">The file to preprocess.</param>
        /// <param name="variables">The variables defined prior to preprocessing.</param>
        /// <returns>XDocument with the postprocessed data.</returns>
        [SuppressMessage("Microsoft.Design", "CA1059:MembersShouldNotExposeCertainConcreteTypes")]
        public XDocument Process(XmlReader reader, IDictionary<string, string> variables, string sourceFile = null)
        {
            if (String.IsNullOrEmpty(sourceFile) && !String.IsNullOrEmpty(reader.BaseURI))
            {
                Uri uri = new Uri(reader.BaseURI);
                sourceFile = uri.AbsolutePath;
            }

            this.core = new PreprocessorCore(this.extensionsByPrefix, this.Message, sourceFile, variables);
            this.core.ResolvedVariableHandler = this.ResolvedVariable;
            this.core.CurrentPlatform = this.currentPlatform;
            this.currentLineNumber = new SourceLineNumber(sourceFile);
            this.currentFileStack.Clear();
            this.currentFileStack.Push(this.core.GetVariableValue(this.currentLineNumber, "sys", "SOURCEFILEDIR"));

            // Process the reader into the output.
            XDocument output = new XDocument();
            try
            {
                foreach (PreprocessorExtension extension in this.extensions)
                {
                    extension.Core = this.core;
                    extension.Initialize();
                }

                this.PreprocessReader(false, reader, output, 0);
            }
            catch (XmlException e)
            {
                this.UpdateCurrentLineNumber(reader, 0);
                throw new WixException(WixErrors.InvalidXml(this.currentLineNumber, "source", e.Message));
            }

            // Fire event with post-processed document.
            ProcessedStreamEventArgs args = new ProcessedStreamEventArgs(sourceFile, output);
            this.OnProcessedStream(args);

            // preprocess the generated XML Document
            foreach (PreprocessorExtension extension in this.extensions)
            {
                extension.PreprocessDocument(output);
            }

            // finalize the preprocessing
            foreach (PreprocessorExtension extension in this.extensions)
            {
                extension.Finish();
                extension.Core = null;
            }

            if (this.core.EncounteredError)
            {
                return null;
            }
            else
            {
                if (null != this.preprocessOut)
                {
                    output.Save(this.preprocessOut);
                    this.preprocessOut.Flush();
                }

                return output;
            }
        }

        /// <summary>
        /// Determins if string is an operator.
        /// </summary>
        /// <param name="operation">String to check.</param>
        /// <returns>true if string is an operator.</returns>
        private static bool IsOperator(string operation)
        {
            if (operation == null)
            {
                return false;
            }

            operation = operation.Trim();
            if (0 == operation.Length)
            {
                return false;
            }

            if ("=" == operation ||
                "!=" == operation ||
                "<" == operation ||
                "<=" == operation ||
                ">" == operation ||
                ">=" == operation ||
                "~=" == operation)
            {
                return true;
            }
            return false;
        }

        /// <summary>
        /// Determines if expression is currently inside quotes.
        /// </summary>
        /// <param name="expression">Expression to evaluate.</param>
        /// <param name="index">Index to start searching in expression.</param>
        /// <returns>true if expression is inside in quotes.</returns>
        private static bool InsideQuotes(string expression, int index)
        {
            if (index == -1)
            {
                return false;
            }

            int numQuotes = 0;
            int tmpIndex = 0;
            while (-1 != (tmpIndex = expression.IndexOf('\"', tmpIndex, index - tmpIndex)))
            {
                numQuotes++;
                tmpIndex++;
            }

            // found an even number of quotes before the index, so we're not inside
            if (numQuotes % 2 == 0)
            {
                return false;
            }

            // found an odd number of quotes, so we are inside
            return true;
        }

        /// <summary>
        /// Fires an event when an ifdef/ifndef directive is processed.
        /// </summary>
        /// <param name="ea">ifdef/ifndef event arguments.</param>
        private void OnIfDef(IfDefEventArgs ea)
        {
            if (null != this.IfDef)
            {
                this.IfDef(this, ea);
            }
        }

        /// <summary>
        /// Fires an event when an included file is processed.
        /// </summary>
        /// <param name="ea">Included file event arguments.</param>
        private void OnIncludedFile(IncludedFileEventArgs ea)
        {
            if (null != this.IncludedFile)
            {
                this.IncludedFile(this, ea);
            }
        }

        /// <summary>
        /// Fires an event after the file is preprocessed.
        /// </summary>
        /// <param name="ea">Included file event arguments.</param>
        private void OnProcessedStream(ProcessedStreamEventArgs ea)
        {
            if (null != this.ProcessedStream)
            {
                this.ProcessedStream(this, ea);
            }
        }

        /// <summary>
        /// Tests expression to see if it starts with a keyword.
        /// </summary>
        /// <param name="expression">Expression to test.</param>
        /// <param name="operation">Operation to test for.</param>
        /// <returns>true if expression starts with a keyword.</returns>
        private static bool StartsWithKeyword(string expression, PreprocessorOperation operation)
        {
            expression = expression.ToUpper(CultureInfo.InvariantCulture);
            switch (operation)
            {
                case PreprocessorOperation.Not:
                    if (expression.StartsWith("NOT ", StringComparison.Ordinal) || expression.StartsWith("NOT(", StringComparison.Ordinal))
                    {
                        return true;
                    }
                    break;
                case PreprocessorOperation.And:
                    if (expression.StartsWith("AND ", StringComparison.Ordinal) || expression.StartsWith("AND(", StringComparison.Ordinal))
                    {
                        return true;
                    }
                    break;
                case PreprocessorOperation.Or:
                    if (expression.StartsWith("OR ", StringComparison.Ordinal) || expression.StartsWith("OR(", StringComparison.Ordinal))
                    {
                        return true;
                    }
                    break;
                default:
                    break;
            }
            return false;
        }

        /// <summary>
        /// Processes an xml reader into an xml writer.
        /// </summary>
        /// <param name="include">Specifies if reader is from an included file.</param>
        /// <param name="reader">Reader for the source document.</param>
        /// <param name="container">Node where content should be added.</param>
        /// <param name="offset">Original offset for the line numbers being processed.</param>
        private void PreprocessReader(bool include, XmlReader reader, XContainer container, int offset)
        {
            XContainer currentContainer = container;
            Stack<XContainer> containerStack = new Stack<XContainer>();

            IfContext ifContext = new IfContext(true, true, IfState.Unknown); // start by assuming we want to keep the nodes in the source code
            Stack<IfContext> ifStack = new Stack<IfContext>();

            // process the reader into the writer
            while (reader.Read())
            {
                // update information here in case an error occurs before the next read
                this.UpdateCurrentLineNumber(reader, offset);

                SourceLineNumber sourceLineNumbers = this.currentLineNumber;

                // check for changes in conditional processing
                if (XmlNodeType.ProcessingInstruction == reader.NodeType)
                {
                    bool ignore = false;
                    string name = null;

                    switch (reader.LocalName)
                    {
                        case "if":
                            ifStack.Push(ifContext);
                            if (ifContext.IsTrue)
                            {
                                ifContext = new IfContext(ifContext.IsTrue & ifContext.Active, this.EvaluateExpression(reader.Value), IfState.If);
                            }
                            else // Use a default IfContext object so we don't try to evaluate the expression if the context isn't true
                            {
                                ifContext = new IfContext();
                            }
                            ignore = true;
                            break;

                        case "ifdef":
                            ifStack.Push(ifContext);
                            name = reader.Value.Trim();
                            if (ifContext.IsTrue)
                            {
                                ifContext = new IfContext(ifContext.IsTrue & ifContext.Active, (null != this.core.GetVariableValue(sourceLineNumbers, name, true)), IfState.If);
                            }
                            else // Use a default IfContext object so we don't try to evaluate the expression if the context isn't true
                            {
                                ifContext = new IfContext();
                            }
                            ignore = true;
                            OnIfDef(new IfDefEventArgs(sourceLineNumbers, true, ifContext.IsTrue, name));
                            break;

                        case "ifndef":
                            ifStack.Push(ifContext);
                            name = reader.Value.Trim();
                            if (ifContext.IsTrue)
                            {
                                ifContext = new IfContext(ifContext.IsTrue & ifContext.Active, (null == this.core.GetVariableValue(sourceLineNumbers, name, true)), IfState.If);
                            }
                            else // Use a default IfContext object so we don't try to evaluate the expression if the context isn't true
                            {
                                ifContext = new IfContext();
                            }
                            ignore = true;
                            OnIfDef(new IfDefEventArgs(sourceLineNumbers, false, !ifContext.IsTrue, name));
                            break;

                        case "elseif":
                            if (0 == ifStack.Count)
                            {
                                throw new WixException(WixErrors.UnmatchedPreprocessorInstruction(this.currentLineNumber, "if", "elseif"));
                            }

                            if (IfState.If != ifContext.IfState && IfState.ElseIf != ifContext.IfState)
                            {
                                throw new WixException(WixErrors.UnmatchedPreprocessorInstruction(this.currentLineNumber, "if", "elseif"));
                            }

                            ifContext.IfState = IfState.ElseIf;   // we're now in an elseif
                            if (!ifContext.WasEverTrue)   // if we've never evaluated the if context to true, then we can try this test
                            {
                                ifContext.IsTrue = this.EvaluateExpression(reader.Value);
                            }
                            else if (ifContext.IsTrue)
                            {
                                ifContext.IsTrue = false;
                            }
                            ignore = true;
                            break;

                        case "else":
                            if (0 == ifStack.Count)
                            {
                                throw new WixException(WixErrors.UnmatchedPreprocessorInstruction(this.currentLineNumber, "if", "else"));
                            }

                            if (IfState.If != ifContext.IfState && IfState.ElseIf != ifContext.IfState)
                            {
                                throw new WixException(WixErrors.UnmatchedPreprocessorInstruction(this.currentLineNumber, "if", "else"));
                            }

                            ifContext.IfState = IfState.Else;   // we're now in an else
                            ifContext.IsTrue = !ifContext.WasEverTrue;   // if we were never true, we can be true now
                            ignore = true;
                            break;

                        case "endif":
                            if (0 == ifStack.Count)
                            {
                                throw new WixException(WixErrors.UnmatchedPreprocessorInstruction(this.currentLineNumber, "if", "endif"));
                            }

                            ifContext = (IfContext)ifStack.Pop();
                            ignore = true;
                            break;
                    }

                    if (ignore)   // ignore this node since we just handled it above
                    {
                        continue;
                    }
                }

                if (!ifContext.Active || !ifContext.IsTrue)   // if our context is not true then skip the rest of the processing and just read the next thing
                {
                    continue;
                }

                switch (reader.NodeType)
                {
                    case XmlNodeType.XmlDeclaration:
                        XDocument document = currentContainer as XDocument;
                        if (null != document)
                        {
                            document.Declaration = new XDeclaration(null, null, null);
                            while (reader.MoveToNextAttribute())
                            {
                                switch (reader.LocalName)
                                {
                                    case "version":
                                        document.Declaration.Version = reader.Value;
                                        break;

                                    case "encoding":
                                        document.Declaration.Encoding = reader.Value;
                                        break;

                                    case "standalone":
                                        document.Declaration.Standalone = reader.Value;
                                        break;
                                }
                            }

                        }
                        //else
                        //{
                        //    display an error? Can this happen?
                        //}
                        break;

                    case XmlNodeType.ProcessingInstruction:
                        switch (reader.LocalName)
                        {
                            case "define":
                                this.PreprocessDefine(reader.Value);
                                break;

                            case "error":
                                this.PreprocessError(reader.Value);
                                break;

                            case "warning":
                                this.PreprocessWarning(reader.Value);
                                break;

                            case "undef":
                                this.PreprocessUndef(reader.Value);
                                break;

                            case "include":
                                this.UpdateCurrentLineNumber(reader, offset);
                                this.PreprocessInclude(reader.Value, currentContainer);
                                break;

                            case "foreach":
                                this.PreprocessForeach(reader, currentContainer, offset);
                                break;

                            case "endforeach": // endforeach is handled in PreprocessForeach, so seeing it here is an error
                                throw new WixException(WixErrors.UnmatchedPreprocessorInstruction(this.currentLineNumber, "foreach", "endforeach"));

                            case "pragma":
                                this.PreprocessPragma(reader.Value, currentContainer);
                                break;

                            default:
                                // unknown processing instructions are currently ignored
                                break;
                        }
                        break;

                    case XmlNodeType.Element:
                        if (0 < this.includeNextStack.Count && this.includeNextStack.Peek())
                        {
                            if ("Include" != reader.LocalName)
                            {
                                this.core.OnMessage(WixErrors.InvalidDocumentElement(this.currentLineNumber, reader.Name, "include", "Include"));
                            }

                            this.includeNextStack.Pop();
                            this.includeNextStack.Push(false);
                            break;
                        }

                        bool empty = reader.IsEmptyElement;
                        XNamespace ns = XNamespace.Get(reader.NamespaceURI);
                        XElement element = new XElement(ns + reader.LocalName);
                        currentContainer.Add(element);

                        this.UpdateCurrentLineNumber(reader, offset);
                        element.AddAnnotation(this.currentLineNumber);

                        while (reader.MoveToNextAttribute())
                        {
                            string value = this.core.PreprocessString(this.currentLineNumber, reader.Value);
                            XNamespace attribNamespace = XNamespace.Get(reader.NamespaceURI);
                            attribNamespace = XNamespace.Xmlns == attribNamespace && reader.LocalName.Equals("xmlns") ? XNamespace.None : attribNamespace;
                            element.Add(new XAttribute(attribNamespace + reader.LocalName, value));
                        }

                        if (!empty)
                        {
                            containerStack.Push(currentContainer);
                            currentContainer = element;
                        }
                        break;

                    case XmlNodeType.EndElement:
                        if (0 < reader.Depth || !include)
                        {
                            currentContainer = containerStack.Pop();
                        }
                        break;

                    case XmlNodeType.Text:
                        string postprocessedText = this.core.PreprocessString(this.currentLineNumber, reader.Value);
                        currentContainer.Add(postprocessedText);
                        break;

                    case XmlNodeType.CDATA:
                        string postprocessedValue = this.core.PreprocessString(this.currentLineNumber, reader.Value);
                        currentContainer.Add(new XCData(postprocessedValue));
                        break;

                    default:
                        break;
                }
            }

            if (0 != ifStack.Count)
            {
                throw new WixException(WixErrors.NonterminatedPreprocessorInstruction(this.currentLineNumber, "if", "endif"));
            }

            // TODO: can this actually happen?
            if (0 != containerStack.Count)
            {
                throw new WixException(WixErrors.NonterminatedPreprocessorInstruction(this.currentLineNumber, "nodes", "nodes"));
            }
        }

        /// <summary>
        /// Processes an error processing instruction.
        /// </summary>
        /// <param name="errorMessage">Text from source.</param>
        private void PreprocessError(string errorMessage)
        {
            SourceLineNumber sourceLineNumbers = this.currentLineNumber;

            // resolve other variables in the error message
            errorMessage = this.core.PreprocessString(sourceLineNumbers, errorMessage);

            throw new WixException(WixErrors.PreprocessorError(sourceLineNumbers, errorMessage));
        }

        /// <summary>
        /// Processes a warning processing instruction.
        /// </summary>
        /// <param name="warningMessage">Text from source.</param>
        private void PreprocessWarning(string warningMessage)
        {
            SourceLineNumber sourceLineNumbers = this.currentLineNumber;

            // resolve other variables in the warning message
            warningMessage = this.core.PreprocessString(sourceLineNumbers, warningMessage);

            this.core.OnMessage(WixWarnings.PreprocessorWarning(sourceLineNumbers, warningMessage));
        }

        /// <summary>
        /// Processes a define processing instruction and creates the appropriate parameter.
        /// </summary>
        /// <param name="originalDefine">Text from source.</param>
        private void PreprocessDefine(string originalDefine)
        {
            Match match = defineRegex.Match(originalDefine);
            SourceLineNumber sourceLineNumbers = this.currentLineNumber;

            if (!match.Success)
            {
                throw new WixException(WixErrors.IllegalDefineStatement(sourceLineNumbers, originalDefine));
            }

            string defineName = match.Groups["varName"].Value;
            string defineValue = match.Groups["varValue"].Value;

            // strip off the optional quotes
            if (1 < defineValue.Length &&
                   ((defineValue.StartsWith("\"", StringComparison.Ordinal) && defineValue.EndsWith("\"", StringComparison.Ordinal))
                || (defineValue.StartsWith("'", StringComparison.Ordinal) && defineValue.EndsWith("'", StringComparison.Ordinal))))
            {
                defineValue = defineValue.Substring(1, defineValue.Length - 2);
            }

            // resolve other variables in the variable value
            defineValue = this.core.PreprocessString(sourceLineNumbers, defineValue);

            if (defineName.StartsWith("var.", StringComparison.Ordinal))
            {
                this.core.AddVariable(sourceLineNumbers, defineName.Substring(4), defineValue);
            }
            else
            {
                this.core.AddVariable(sourceLineNumbers, defineName, defineValue);
            }
        }

        /// <summary>
        /// Processes an undef processing instruction and creates the appropriate parameter.
        /// </summary>
        /// <param name="originalDefine">Text from source.</param>
        private void PreprocessUndef(string originalDefine)
        {
            SourceLineNumber sourceLineNumbers = this.currentLineNumber;
            string name = this.core.PreprocessString(sourceLineNumbers, originalDefine.Trim());

            if (name.StartsWith("var.", StringComparison.Ordinal))
            {
                this.core.RemoveVariable(sourceLineNumbers, name.Substring(4));
            }
            else
            {
                this.core.RemoveVariable(sourceLineNumbers, name);
            }
        }

        /// <summary>
        /// Processes an included file.
        /// </summary>
        /// <param name="includePath">Path to included file.</param>
        /// <param name="parent">Parent container for included content.</param>
        private void PreprocessInclude(string includePath, XContainer parent)
        {
            SourceLineNumber sourceLineNumbers = this.currentLineNumber;

            // preprocess variables in the path
            includePath = this.core.PreprocessString(sourceLineNumbers, includePath);

            string includeFile = this.GetIncludeFile(includePath);

            if (null == includeFile)
            {
                throw new WixException(WixErrors.FileNotFound(sourceLineNumbers, includePath, "include"));
            }

            using (XmlReader reader = XmlReader.Create(includeFile, DocumentXmlReaderSettings))
            {
                this.PushInclude(includeFile);

                // process the included reader into the writer
                try
                {
                    this.PreprocessReader(true, reader, parent, 0);
                }
                catch (XmlException e)
                {
                    this.UpdateCurrentLineNumber(reader, 0);
                    throw new WixException(WixErrors.InvalidXml(sourceLineNumbers, "source", e.Message));
                }

                this.OnIncludedFile(new IncludedFileEventArgs(sourceLineNumbers, includeFile));

                this.PopInclude();
            }
        }

        /// <summary>
        /// Preprocess a foreach processing instruction.
        /// </summary>
        /// <param name="reader">The xml reader.</param>
        /// <param name="container">The container where to output processed data.</param>
        /// <param name="offset">Offset for the line numbers.</param>
        private void PreprocessForeach(XmlReader reader, XContainer container, int offset)
        {
            // find the "in" token
            int indexOfInToken = reader.Value.IndexOf(" in ", StringComparison.Ordinal);
            if (0 > indexOfInToken)
            {
                throw new WixException(WixErrors.IllegalForeach(this.currentLineNumber, reader.Value));
            }

            // parse out the variable name
            string varName = reader.Value.Substring(0, indexOfInToken).Trim();
            string varValuesString = reader.Value.Substring(indexOfInToken + 4).Trim();

            // preprocess the variable values string because it might be a variable itself
            varValuesString = this.core.PreprocessString(this.currentLineNumber, varValuesString);

            string[] varValues = varValuesString.Split(';');

            // go through all the empty strings
            while (reader.Read() && XmlNodeType.Whitespace == reader.NodeType)
            {
            }

            // get the offset of this xml fragment (for some reason its always off by 1)
            IXmlLineInfo lineInfoReader = reader as IXmlLineInfo;
            if (null != lineInfoReader)
            {
                offset += lineInfoReader.LineNumber - 1;
            }

            XmlTextReader textReader = reader as XmlTextReader;
            // dump the xml to a string (maintaining whitespace if possible)
            if (null != textReader)
            {
                textReader.WhitespaceHandling = WhitespaceHandling.All;
            }

            StringBuilder fragmentBuilder = new StringBuilder();
            int nestedForeachCount = 1;
            while (nestedForeachCount != 0)
            {
                if (reader.NodeType == XmlNodeType.ProcessingInstruction)
                {
                    switch (reader.LocalName)
                    {
                        case "foreach":
                            ++nestedForeachCount;
                            // Output the foreach statement
                            fragmentBuilder.AppendFormat("<?foreach {0}?>", reader.Value);
                            break;

                        case "endforeach":
                            --nestedForeachCount;
                            if (0 != nestedForeachCount)
                            {
                                fragmentBuilder.Append("<?endforeach ?>");
                            }
                            break;

                        default:
                            fragmentBuilder.AppendFormat("<?{0} {1}?>", reader.LocalName, reader.Value);
                            break;
                    }
                }
                else if (reader.NodeType == XmlNodeType.Element)
                {
                    fragmentBuilder.Append(reader.ReadOuterXml());
                    continue;
                }
                else if (reader.NodeType == XmlNodeType.Whitespace)
                {
                    // Or output the whitespace
                    fragmentBuilder.Append(reader.Value);
                }
                else if (reader.NodeType == XmlNodeType.None)
                {
                    throw new WixException(WixErrors.ExpectedEndforeach(this.currentLineNumber));
                }

                reader.Read();
            }

            using (MemoryStream fragmentStream = new MemoryStream(Encoding.UTF8.GetBytes(fragmentBuilder.ToString())))
            using (XmlReader loopReader = XmlReader.Create(fragmentStream, FragmentXmlReaderSettings))
            {
                // process each iteration, updating the variable's value each time
                foreach (string varValue in varValues)
                {
                    // Always overwrite foreach variables.
                    this.core.AddVariable(this.currentLineNumber, varName, varValue, false);

                    try
                    {
                        this.PreprocessReader(false, loopReader, container, offset);
                    }
                    catch (XmlException e)
                    {
                        this.UpdateCurrentLineNumber(loopReader, offset);
                        throw new WixException(WixErrors.InvalidXml(this.currentLineNumber, "source", e.Message));
                    }

                    fragmentStream.Position = 0; // seek back to the beginning for the next loop.
                }
            }
        }

        /// <summary>
        /// Processes a pragma processing instruction
        /// </summary>
        /// <param name="pragmaText">Text from source.</param>
        private void PreprocessPragma(string pragmaText, XContainer parent)
        {
            Match match = pragmaRegex.Match(pragmaText);
            SourceLineNumber sourceLineNumbers = this.currentLineNumber;

            if (!match.Success)
            {
                throw new WixException(WixErrors.InvalidPreprocessorPragma(sourceLineNumbers, pragmaText));
            }

            // resolve other variables in the pragma argument(s)
            string pragmaArgs = this.core.PreprocessString(sourceLineNumbers, match.Groups["pragmaValue"].Value).Trim();

            try
            {
                this.core.PreprocessPragma(sourceLineNumbers, match.Groups["pragmaName"].Value.Trim(), pragmaArgs, parent);
            }
            catch (Exception e)
            {
                throw new WixException(WixErrors.PreprocessorExtensionPragmaFailed(sourceLineNumbers, pragmaText, e.Message));
            }
        }

        /// <summary>
        /// Gets the next token in an expression.
        /// </summary>
        /// <param name="originalExpression">Expression to parse.</param>
        /// <param name="expression">Expression with token removed.</param>
        /// <param name="stringLiteral">Flag if token is a string literal instead of a variable.</param>
        /// <returns>Next token.</returns>
        private string GetNextToken(string originalExpression, ref string expression, out bool stringLiteral)
        {
            stringLiteral = false;
            string token = String.Empty;
            expression = expression.Trim();
            if (0 == expression.Length)
            {
                return String.Empty;
            }

            if (expression.StartsWith("\"", StringComparison.Ordinal))
            {
                stringLiteral = true;
                int endingQuotes = expression.IndexOf('\"', 1);
                if (-1 == endingQuotes)
                {
                    throw new WixException(WixErrors.UnmatchedQuotesInExpression(this.currentLineNumber, originalExpression));
                }

                // cut the quotes off the string
                token = this.core.PreprocessString(this.currentLineNumber, expression.Substring(1, endingQuotes - 1));

                // advance past this string
                expression = expression.Substring(endingQuotes + 1).Trim();
            }
            else if (expression.StartsWith("$(", StringComparison.Ordinal))
            {
                // Find the ending paren of the expression
                int endingParen = -1;
                int openedCount = 1;
                for (int i = 2; i < expression.Length; i++)
                {
                    if ('(' == expression[i])
                    {
                        openedCount++;
                    }
                    else if (')' == expression[i])
                    {
                        openedCount--;
                    }

                    if (openedCount == 0)
                    {
                        endingParen = i;
                        break;
                    }
                }

                if (-1 == endingParen)
                {
                    throw new WixException(WixErrors.UnmatchedParenthesisInExpression(this.currentLineNumber, originalExpression));
                }
                token = expression.Substring(0, endingParen + 1);

                // Advance past this variable
                expression = expression.Substring(endingParen + 1).Trim();
            }
            else
            {
                // Cut the token off at the next equal, space, inequality operator,
                // or end of string, whichever comes first
                int space = expression.IndexOf(" ", StringComparison.Ordinal);
                int equals = expression.IndexOf("=", StringComparison.Ordinal);
                int lessThan = expression.IndexOf("<", StringComparison.Ordinal);
                int lessThanEquals = expression.IndexOf("<=", StringComparison.Ordinal);
                int greaterThan = expression.IndexOf(">", StringComparison.Ordinal);
                int greaterThanEquals = expression.IndexOf(">=", StringComparison.Ordinal);
                int notEquals = expression.IndexOf("!=", StringComparison.Ordinal);
                int equalsNoCase = expression.IndexOf("~=", StringComparison.Ordinal);
                int closingIndex;

                if (space == -1)
                {
                    space = Int32.MaxValue;
                }

                if (equals == -1)
                {
                    equals = Int32.MaxValue;
                }

                if (lessThan == -1)
                {
                    lessThan = Int32.MaxValue;
                }

                if (lessThanEquals == -1)
                {
                    lessThanEquals = Int32.MaxValue;
                }

                if (greaterThan == -1)
                {
                    greaterThan = Int32.MaxValue;
                }

                if (greaterThanEquals == -1)
                {
                    greaterThanEquals = Int32.MaxValue;
                }

                if (notEquals == -1)
                {
                    notEquals = Int32.MaxValue;
                }

                if (equalsNoCase == -1)
                {
                    equalsNoCase = Int32.MaxValue;
                }

                closingIndex = Math.Min(space, Math.Min(equals, Math.Min(lessThan, Math.Min(lessThanEquals, Math.Min(greaterThan, Math.Min(greaterThanEquals, Math.Min(equalsNoCase, notEquals)))))));

                if (Int32.MaxValue == closingIndex)
                {
                    closingIndex = expression.Length;
                }

                // If the index is 0, we hit an operator, so return it
                if (0 == closingIndex)
                {
                    // Length 2 operators
                    if (closingIndex == lessThanEquals || closingIndex == greaterThanEquals || closingIndex == notEquals || closingIndex == equalsNoCase)
                    {
                        closingIndex = 2;
                    }
                    else // Length 1 operators
                    {
                        closingIndex = 1;
                    }
                }

                // Cut out the new token
                token = expression.Substring(0, closingIndex).Trim();
                expression = expression.Substring(closingIndex).Trim();
            }

            return token;
        }

        /// <summary>
        /// Gets the value for a variable.
        /// </summary>
        /// <param name="originalExpression">Original expression for error message.</param>
        /// <param name="variable">Variable to evaluate.</param>
        /// <returns>Value of variable.</returns>
        private string EvaluateVariable(string originalExpression, string variable)
        {
            // By default it's a literal and will only be evaluated if it
            // matches the variable format
            string varValue = variable;

            if (variable.StartsWith("$(", StringComparison.Ordinal))
            {
                try
                {
                    varValue = this.core.PreprocessString(this.currentLineNumber, variable);
                }
                catch (ArgumentNullException)
                {
                    // non-existent variables are expected
                    varValue = null;
                }
            }
            else if (variable.IndexOf("(", StringComparison.Ordinal) != -1 || variable.IndexOf(")", StringComparison.Ordinal) != -1)
            {
                // make sure it doesn't contain parenthesis
                throw new WixException(WixErrors.UnmatchedParenthesisInExpression(this.currentLineNumber, originalExpression));
            }
            else if (variable.IndexOf("\"", StringComparison.Ordinal) != -1)
            {
                // shouldn't contain quotes
                throw new WixException(WixErrors.UnmatchedQuotesInExpression(this.currentLineNumber, originalExpression));
            }

            return varValue;
        }

        /// <summary>
        /// Gets the left side value, operator, and right side value of an expression.
        /// </summary>
        /// <param name="originalExpression">Original expression to evaluate.</param>
        /// <param name="expression">Expression modified while processing.</param>
        /// <param name="leftValue">Left side value from expression.</param>
        /// <param name="operation">Operation in expression.</param>
        /// <param name="rightValue">Right side value from expression.</param>
        private void GetNameValuePair(string originalExpression, ref string expression, out string leftValue, out string operation, out string rightValue)
        {
            bool stringLiteral;
            leftValue = this.GetNextToken(originalExpression, ref expression, out stringLiteral);

            // If it wasn't a string literal, evaluate it
            if (!stringLiteral)
            {
                leftValue = this.EvaluateVariable(originalExpression, leftValue);
            }

            // Get the operation
            operation = this.GetNextToken(originalExpression, ref expression, out stringLiteral);
            if (IsOperator(operation))
            {
                if (stringLiteral)
                {
                    throw new WixException(WixErrors.UnmatchedQuotesInExpression(this.currentLineNumber, originalExpression));
                }

                rightValue = this.GetNextToken(originalExpression, ref expression, out stringLiteral);

                // If it wasn't a string literal, evaluate it
                if (!stringLiteral)
                {
                    rightValue = this.EvaluateVariable(originalExpression, rightValue);
                }
            }
            else
            {
                // Prepend the token back on the expression since it wasn't an operator
                // and put the quotes back on the literal if necessary

                if (stringLiteral)
                {
                    operation = "\"" + operation + "\"";
                }
                expression = (operation + " " + expression).Trim();

                // If no operator, just check for existence
                operation = "";
                rightValue = "";
            }
        }

        /// <summary>
        /// Evaluates an expression.
        /// </summary>
        /// <param name="originalExpression">Original expression to evaluate.</param>
        /// <param name="expression">Expression modified while processing.</param>
        /// <returns>true if expression evaluates to true.</returns>
        private bool EvaluateAtomicExpression(string originalExpression, ref string expression)
        {
            // Quick test to see if the first token is a variable
            bool startsWithVariable = expression.StartsWith("$(", StringComparison.Ordinal);

            string leftValue;
            string rightValue;
            string operation;
            this.GetNameValuePair(originalExpression, ref expression, out leftValue, out operation, out rightValue);

            bool expressionValue = false;

            // If the variables don't exist, they were evaluated to null
            if (null == leftValue || null == rightValue)
            {
                if (operation.Length > 0)
                {
                    throw new WixException(WixErrors.ExpectedVariable(this.currentLineNumber, originalExpression));
                }

                // false expression
            }
            else if (operation.Length == 0)
            {
                // There is no right side of the equation.
                // If the variable was evaluated, it exists, so the expression is true
                if (startsWithVariable)
                {
                    expressionValue = true;
                }
                else
                {
                    throw new WixException(WixErrors.UnexpectedLiteral(this.currentLineNumber, originalExpression));
                }
            }
            else
            {
                leftValue = leftValue.Trim();
                rightValue = rightValue.Trim();
                if ("=" == operation)
                {
                    if (leftValue == rightValue)
                    {
                        expressionValue = true;
                    }
                }
                else if ("!=" == operation)
                {
                    if (leftValue != rightValue)
                    {
                        expressionValue = true;
                    }
                }
                else if ("~=" == operation)
                {
                    if (String.Equals(leftValue, rightValue, StringComparison.OrdinalIgnoreCase))
                    {
                        expressionValue = true;
                    }
                }
                else
                {
                    // Convert the numbers from strings
                    int rightInt;
                    int leftInt;
                    try
                    {
                        rightInt = Int32.Parse(rightValue, CultureInfo.InvariantCulture);
                        leftInt = Int32.Parse(leftValue, CultureInfo.InvariantCulture);
                    }
                    catch (FormatException)
                    {
                        throw new WixException(WixErrors.IllegalIntegerInExpression(this.currentLineNumber, originalExpression));
                    }
                    catch (OverflowException)
                    {
                        throw new WixException(WixErrors.IllegalIntegerInExpression(this.currentLineNumber, originalExpression));
                    }

                    // Compare the numbers
                    if ("<" == operation && leftInt < rightInt ||
                        "<=" == operation && leftInt <= rightInt ||
                        ">" == operation && leftInt > rightInt ||
                        ">=" == operation && leftInt >= rightInt)
                    {
                        expressionValue = true;
                    }
                }
            }

            return expressionValue;
        }

        /// <summary>
        /// Gets a sub-expression in parenthesis.
        /// </summary>
        /// <param name="originalExpression">Original expression to evaluate.</param>
        /// <param name="expression">Expression modified while processing.</param>
        /// <param name="endSubExpression">Index of end of sub-expression.</param>
        /// <returns>Sub-expression in parenthesis.</returns>
        private string GetParenthesisExpression(string originalExpression, string expression, out int endSubExpression)
        {
            endSubExpression = 0;

            // if the expression doesn't start with parenthesis, leave it alone
            if (!expression.StartsWith("(", StringComparison.Ordinal))
            {
                return expression;
            }

            // search for the end of the expression with the matching paren
            int openParenIndex = 0;
            int closeParenIndex = 1;
            while (openParenIndex != -1 && openParenIndex < closeParenIndex)
            {
                closeParenIndex = expression.IndexOf(')', closeParenIndex);
                if (closeParenIndex == -1)
                {
                    throw new WixException(WixErrors.UnmatchedParenthesisInExpression(this.currentLineNumber, originalExpression));
                }

                if (InsideQuotes(expression, closeParenIndex))
                {
                    // ignore stuff inside quotes (it's a string literal)
                }
                else
                {
                    // Look to see if there is another open paren before the close paren
                    // and skip over the open parens while they are in a string literal
                    do
                    {
                        openParenIndex++;
                        openParenIndex = expression.IndexOf('(', openParenIndex, closeParenIndex - openParenIndex);
                    }
                    while (InsideQuotes(expression, openParenIndex));
                }

                // Advance past the closing paren
                closeParenIndex++;
            }

            endSubExpression = closeParenIndex;

            // Return the expression minus the parenthesis
            return expression.Substring(1, closeParenIndex - 2);
        }

        /// <summary>
        /// Updates expression based on operation.
        /// </summary>
        /// <param name="currentValue">State to update.</param>
        /// <param name="operation">Operation to apply to current value.</param>
        /// <param name="prevResult">Previous result.</param>
        private void UpdateExpressionValue(ref bool currentValue, PreprocessorOperation operation, bool prevResult)
        {
            switch (operation)
            {
                case PreprocessorOperation.And:
                    currentValue = currentValue && prevResult;
                    break;
                case PreprocessorOperation.Or:
                    currentValue = currentValue || prevResult;
                    break;
                case PreprocessorOperation.Not:
                    currentValue = !currentValue;
                    break;
                default:
                    throw new WixException(WixErrors.UnexpectedPreprocessorOperator(this.currentLineNumber, operation.ToString()));
            }
        }

        /// <summary>
        /// Evaluate an expression.
        /// </summary>
        /// <param name="expression">Expression to evaluate.</param>
        /// <returns>Boolean result of expression.</returns>
        private bool EvaluateExpression(string expression)
        {
            string tmpExpression = expression;
            return this.EvaluateExpressionRecurse(expression, ref tmpExpression, PreprocessorOperation.And, true);
        }

        /// <summary>
        /// Recurse through the expression to evaluate if it is true or false.
        /// The expression is evaluated left to right. 
        /// The expression is case-sensitive (converted to upper case) with the
        /// following exceptions: variable names and keywords (and, not, or).
        /// Comparisons with = and != are string comparisons.  
        /// Comparisons with inequality operators must be done on valid integers.
        /// 
        /// The operator precedence is:
        ///    ""
        ///    ()
        ///    &lt;, &gt;, &lt;=, &gt;=, =, !=
        ///    Not
        ///    And, Or
        ///    
        /// Valid expressions include:
        ///   not $(var.B) or not $(var.C)
        ///   (($(var.A))and $(var.B) ="2")or Not((($(var.C))) and $(var.A))
        ///   (($(var.A)) and $(var.B) = " 3 ") or $(var.C)
        ///   $(var.A) and $(var.C) = "3" or $(var.C) and $(var.D) = $(env.windir)
        ///   $(var.A) and $(var.B)>2 or $(var.B) &lt;= 2
        ///   $(var.A) != "2" 
        /// </summary>
        /// <param name="originalExpression">The original expression</param>
        /// <param name="expression">The expression currently being evaluated</param>
        /// <param name="prevResultOperation">The operation to apply to this result</param>
        /// <param name="prevResult">The previous result to apply to this result</param>
        /// <returns>Boolean to indicate if the expression is true or false</returns>
        private bool EvaluateExpressionRecurse(string originalExpression, ref string expression, PreprocessorOperation prevResultOperation, bool prevResult)
        {
            bool expressionValue = false;
            expression = expression.Trim();
            if (expression.Length == 0)
            {
                throw new WixException(WixErrors.UnexpectedEmptySubexpression(this.currentLineNumber, originalExpression));
            }

            // If the expression starts with parenthesis, evaluate it
            if (expression.IndexOf('(') == 0)
            {
                int endSubExpressionIndex;
                string subExpression = this.GetParenthesisExpression(originalExpression, expression, out endSubExpressionIndex);
                expressionValue = this.EvaluateExpressionRecurse(originalExpression, ref subExpression, PreprocessorOperation.And, true);

                // Now get the rest of the expression that hasn't been evaluated
                expression = expression.Substring(endSubExpressionIndex).Trim();
            }
            else
            {
                // Check for NOT
                if (StartsWithKeyword(expression, PreprocessorOperation.Not))
                {
                    expression = expression.Substring(3).Trim();
                    if (expression.Length == 0)
                    {
                        throw new WixException(WixErrors.ExpectedExpressionAfterNot(this.currentLineNumber, originalExpression));
                    }

                    expressionValue = this.EvaluateExpressionRecurse(originalExpression, ref expression, PreprocessorOperation.Not, true);
                }
                else // Expect a literal
                {
                    expressionValue = this.EvaluateAtomicExpression(originalExpression, ref expression);

                    // Expect the literal that was just evaluated to already be cut off
                }
            }
            this.UpdateExpressionValue(ref expressionValue, prevResultOperation, prevResult);

            // If there's still an expression left, it must start with AND or OR.
            if (expression.Trim().Length > 0)
            {
                if (StartsWithKeyword(expression, PreprocessorOperation.And))
                {
                    expression = expression.Substring(3);
                    return this.EvaluateExpressionRecurse(originalExpression, ref expression, PreprocessorOperation.And, expressionValue);
                }
                else if (StartsWithKeyword(expression, PreprocessorOperation.Or))
                {
                    expression = expression.Substring(2);
                    return this.EvaluateExpressionRecurse(originalExpression, ref expression, PreprocessorOperation.Or, expressionValue);
                }
                else
                {
                    throw new WixException(WixErrors.InvalidSubExpression(this.currentLineNumber, expression, originalExpression));
                }
            }

            return expressionValue;
        }

        /// <summary>
        /// Update the current line number with the reader's current state.
        /// </summary>
        /// <param name="reader">The xml reader for the preprocessor.</param>
        /// <param name="offset">This is the artificial offset of the line numbers from the reader.  Used for the foreach processing.</param>
        private void UpdateCurrentLineNumber(XmlReader reader, int offset)
        {
            IXmlLineInfo lineInfoReader = reader as IXmlLineInfo;
            if (null != lineInfoReader)
            {
                int newLine = lineInfoReader.LineNumber + offset;

                if (this.currentLineNumber.LineNumber != newLine)
                {
                    this.currentLineNumber = new SourceLineNumber(this.currentLineNumber.FileName, newLine);
                }
            }
        }

        /// <summary>
        /// Pushes a file name on the stack of included files.
        /// </summary>
        /// <param name="fileName">Name to push on to the stack of included files.</param>
        private void PushInclude(string fileName)
        {
            this.currentFileStack.Push(fileName);
            this.sourceStack.Push(this.currentLineNumber);
            this.currentLineNumber = new SourceLineNumber(fileName);
            this.includeNextStack.Push(true);
        }

        /// <summary>
        /// Pops a file name from the stack of included files.
        /// </summary>
        private void PopInclude()
        {
            this.currentLineNumber = this.sourceStack.Pop();

            this.currentFileStack.Pop();
            this.includeNextStack.Pop();
        }

        /// <summary>
        /// Go through search paths, looking for a matching include file.
        /// Start the search in the directory of the source file, then go
        /// through the search paths in the order given on the command line
        /// (leftmost first, ...).
        /// </summary>
        /// <param name="includePath">User-specified path to the included file (usually just the file name).</param>
        /// <returns>Returns a FileInfo for the found include file, or null if the file cannot be found.</returns>
        private string GetIncludeFile(string includePath)
        {
            string finalIncludePath = null;

            includePath = includePath.Trim();

            // remove quotes (only if they match)
            if ((includePath.StartsWith("\"", StringComparison.Ordinal) && includePath.EndsWith("\"", StringComparison.Ordinal)) ||
                (includePath.StartsWith("'", StringComparison.Ordinal) && includePath.EndsWith("'", StringComparison.Ordinal)))
            {
                includePath = includePath.Substring(1, includePath.Length - 2);
            }

            // check if the include file is a full path
            if (Path.IsPathRooted(includePath))
            {
                if (File.Exists(includePath))
                {
                    finalIncludePath = includePath;
                }
            }
            else // relative path
            {
                // build a string to test the directory containing the source file first
                string currentFolder = this.currentFileStack.Peek();
                string includeTestPath = Path.Combine(Path.GetDirectoryName(currentFolder) ?? String.Empty, includePath);

                // test the source file directory
                if (File.Exists(includeTestPath))
                {
                    finalIncludePath = includeTestPath;
                }
                else // test all search paths in the order specified on the command line
                {
                    foreach (string includeSearchPath in this.IncludeSearchPaths)
                    {
                        // if the path exists, we have found the final string
                        includeTestPath = Path.Combine(includeSearchPath, includePath);
                        if (File.Exists(includeTestPath))
                        {
                            finalIncludePath = includeTestPath;
                            break;
                        }
                    }
                }
            }

            return finalIncludePath;
        }
    }
}
