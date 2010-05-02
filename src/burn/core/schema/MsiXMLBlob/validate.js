
/* how to get cmdline args 
var argsColl = WScript.Arguments;
var enumArgs = new Enumerator (argsColl);
for (enumArgs.moveFirst(); !enumArgs.atEnd(); enumArgs.moveNext()) {
    var objArg = enumArgs.item();
    if (objArg !=  null)
        WScript.Echo ("" + objArg);
}
*/


Main();
WScript.Echo ("Ending validation");

function Main()
{
    // get cmdline args:  validate.js xmlFile xsdFile
    var argsColl = WScript.Arguments;
    var enumArgs = new Enumerator (argsColl);
    if (enumArgs.atEnd()) {
        WScript.Echo ("bad command line:  validate.js xmlFile xsdFile");
        return;
    }
    var xmlFile = enumArgs.item();
    enumArgs.moveNext();
    if (enumArgs.atEnd()) {
        WScript.Echo ("bad command line:  validate.js xmlFile xsdFile");
        return;
    }
    var xsdFile = enumArgs.item();

    WScript.Echo ("Starting validation on " + xmlFile + " and " + xsdFile);

    // Load XML document
    var myXmlDoc = new ActiveXObject("Msxml2.DOMDocument.4.0");

    myXmlDoc.async = false;
    myXmlDoc.resolveExternals = false;

    if (myXmlDoc.load (xmlFile))
    {
        WScript.Echo (xmlFile + " loaded");
    }
    else
    {
        WScript.Echo ("could not load " + xmlFile);
        return;
    }

    // Create a Schema Cache
    var mySchemaCache = new ActiveXObject ("Msxml2.XMLSchemaCache.4.0");

    try
    {
        mySchemaCache.add ("http://www.microsoft.com/msi/patch_applicability.xsd", xsdFile);
        WScript.Echo (xsdFile + " loaded into cache.");
    }
    catch (e)
    {
        WScript.Echo ("could NOT load " + xsdFile + " into cache: " + e.message + "");
        return;
    }

    try
    {
        myXmlDoc.schemas = mySchemaCache;
        WScript.Echo ("mySchemaCache added into myXmlDoc ");
    }
    catch (e)
    {
        WScript.Echo ("could NOT add mySchemaCache to myXmlDoc: " + e.message + "");
        return;
    }

    try
    {
        var err = myXmlDoc.validate ();
        if (err.errorCode == 0)
        {
            WScript.Echo ("VALIDATED!!!");
        }
        else
        {
            WScript.Echo ("FAILURE TO VALIDATE:  " + err.reason + "");
            WScript.Echo ("FAILURE ON LINE    :  " + err.line + "");
            WScript.Echo ("BAD TEXT           :  " + err.srcText + "");
        }
    }
    catch (e)
    {
        WScript.Echo ("could NOT validate myXmlDoc: " + e.message + "");
    }
}
