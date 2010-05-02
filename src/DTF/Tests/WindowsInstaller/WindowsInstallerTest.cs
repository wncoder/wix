namespace Microsoft.Deployment.Test
{
    using System;
    using System.IO;
    using System.Windows.Forms;
    using System.Globalization;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Deployment.WindowsInstaller;
    using View = Microsoft.Deployment.WindowsInstaller.View;

    [TestClass]
    public class WindowsInstallerTest
    {
        public WindowsInstallerTest()
        {
        }

        [TestInitialize()]
        public void Initialize()
        {
        }

        [TestCleanup()]
        public void Cleanup()
        {
        }

        [TestMethod]
        public void InstallerErrorMessages()
        {
            string msg3002 = Installer.GetErrorMessage(3002);
            Console.WriteLine("3002=" + msg3002);
            Assert.IsNotNull(msg3002);
            Assert.IsTrue(msg3002.Length > 0);
        }

        [TestMethod]
        public void InstallerDatabaseSchema()
        {
            string dbFile = "InstallerDatabaseSchema.msi";

            using (Database db = new Database(dbFile, DatabaseOpenMode.CreateDirect))
            {
                WindowsInstallerUtils.InitializeProductDatabase(db);
                db.Commit();
            }

            Assert.IsTrue(File.Exists(dbFile), "Checking whether created database file " + dbFile + " exists.");

            using (Database db = new Database(dbFile, DatabaseOpenMode.ReadOnly))
            {
                TableCollection tables = db.Tables;
                Assert.AreEqual<int>(Schema.Tables.Count, tables.Count, "Counting tables.");
                Assert.AreEqual<int>(Schema.Property.Columns.Count, tables["Property"].Columns.Count, "Counting columns in Property table.");

                foreach (TableInfo tableInfo in tables)
                {
                    Console.WriteLine(tableInfo.Name);
                    foreach (ColumnInfo columnInfo in tableInfo.Columns)
                    {
                        Console.WriteLine("\t{0} {1}", columnInfo.Name, columnInfo.ColumnDefinitionString);
                    }
                }
            }
        }

        [TestMethod]
        public void InstallerViewTables()
        {
            string dbFile = "InstallerViewTables.msi";

            using (Database db = new Database(dbFile, DatabaseOpenMode.CreateDirect))
            {
                WindowsInstallerUtils.InitializeProductDatabase(db);
                db.Commit();

                using (View view1 = db.OpenView("SELECT `Property`, `Value` FROM `Property` WHERE `Value` IS NOT NULL"))
                {
                    IList<TableInfo> viewTables = view1.Tables;
                    Assert.IsNotNull(viewTables);
                    Assert.AreEqual<int>(1, viewTables.Count);
                    Assert.AreEqual<String>("Property", viewTables[0].Name);
                }

                using (View view2 = db.OpenView("INSERT INTO `Property` (`Property`, `Value`) VALUES ('TestViewTables', 1)"))
                {
                    IList<TableInfo> viewTables = view2.Tables;
                    Assert.IsNotNull(viewTables);
                    Assert.AreEqual<int>(1, viewTables.Count);
                    Assert.AreEqual<String>("Property", viewTables[0].Name);
                }

                using (View view3 = db.OpenView("UPDATE `Property` SET `Value` = 2 WHERE `Property` = 'TestViewTables'"))
                {
                    IList<TableInfo> viewTables = view3.Tables;
                    Assert.IsNotNull(viewTables);
                    Assert.AreEqual<int>(1, viewTables.Count);
                    Assert.AreEqual<String>("Property", viewTables[0].Name);
                }

                using (View view4 = db.OpenView("alter table Property hold"))
                {
                    IList<TableInfo> viewTables = view4.Tables;
                    Assert.IsNotNull(viewTables);
                    Assert.AreEqual<int>(1, viewTables.Count);
                    Assert.AreEqual<String>("Property", viewTables[0].Name);
                }
            }
        }

        [TestMethod]
        public void InstallerInstallProduct()
        {
            string dbFile = "InstallerInstallProduct.msi";
            string productCode;

            using (Database db = new Database(dbFile, DatabaseOpenMode.CreateDirect))
            {
                WindowsInstallerUtils.InitializeProductDatabase(db);
                WindowsInstallerUtils.CreateTestProduct(db);

                productCode = db.ExecuteStringQuery("SELECT `Value` FROM `Property` WHERE `Property` = 'ProductCode'")[0];

                db.Commit();
            }

            ProductInstallation installation = new ProductInstallation(productCode);
            Assert.IsFalse(installation.IsInstalled, "Checking that product is not installed before starting.");

            Installer.SetInternalUI(InstallUIOptions.Silent);
            ExternalUIHandler prevHandler = Installer.SetExternalUI(WindowsInstallerTest.ExternalUILogger,
                InstallLogModes.FatalExit |
                InstallLogModes.Error |
                InstallLogModes.Warning |
                InstallLogModes.User |
                InstallLogModes.Info |
                InstallLogModes.ResolveSource |
                InstallLogModes.OutOfDiskSpace |
                InstallLogModes.ActionStart |
                InstallLogModes.ActionData |
                InstallLogModes.CommonData |
                InstallLogModes.Progress |
                InstallLogModes.Initialize |
                InstallLogModes.Terminate |
                InstallLogModes.ShowDialog);
            Assert.IsNull(prevHandler, "Checking that returned previous UI handler is null.");

            Exception caughtEx = null;
            try
            {
                Installer.InstallProduct(dbFile, String.Empty);
            }
            catch (Exception ex) { caughtEx = ex; }
            Assert.IsNull(caughtEx, "Exception thrown while installing product: " + caughtEx);

            prevHandler = Installer.SetExternalUI(prevHandler, InstallLogModes.None);
            Assert.AreEqual<ExternalUIHandler>(WindowsInstallerTest.ExternalUILogger, prevHandler, "Checking that previously-set UI handler is returned.");

            Assert.IsTrue(installation.IsInstalled, "Checking that product is installed.");
            Console.WriteLine();
            Console.WriteLine();
            Console.WriteLine();
            Console.WriteLine("===================================================================");
            Console.WriteLine();
            Console.WriteLine();
            Console.WriteLine();

            ExternalUIRecordHandler prevRecHandler = Installer.SetExternalUI(WindowsInstallerTest.ExternalUIRecordLogger,
                InstallLogModes.FatalExit |
                InstallLogModes.Error |
                InstallLogModes.Warning |
                InstallLogModes.User |
                InstallLogModes.Info |
                InstallLogModes.ResolveSource |
                InstallLogModes.OutOfDiskSpace |
                InstallLogModes.ActionStart |
                InstallLogModes.ActionData |
                InstallLogModes.CommonData |
                InstallLogModes.Progress |
                InstallLogModes.Initialize |
                InstallLogModes.Terminate |
                InstallLogModes.ShowDialog);
            Assert.IsNull(prevRecHandler, "Checking that returned previous UI record handler is null.");

            try
            {
                Installer.InstallProduct(dbFile, "REMOVE=All");
            }
            catch (Exception ex) { caughtEx = ex; }
            Assert.IsNull(caughtEx, "Exception thrown while installing product: " + caughtEx);

            Assert.IsFalse(installation.IsInstalled, "Checking that product is not installed after removing.");

            prevRecHandler = Installer.SetExternalUI(prevRecHandler, InstallLogModes.None);
            Assert.AreEqual<ExternalUIRecordHandler>(WindowsInstallerTest.ExternalUIRecordLogger, prevRecHandler, "Checking that previously-set UI record handler is returned.");
        }

        public static MessageResult ExternalUILogger(
            InstallMessage messageType,
            string message,
            MessageBoxButtons buttons,
            MessageBoxIcon icon,
            MessageBoxDefaultButton defaultButton)
        {
            Console.WriteLine("{0}: {1}", messageType, message);
            return MessageResult.None;
        }

        public static MessageResult ExternalUIRecordLogger(
            InstallMessage messageType,
            Record messageRecord,
            MessageBoxButtons buttons,
            MessageBoxIcon icon,
            MessageBoxDefaultButton defaultButton)
        {
            if (messageRecord != null)
            {
                if (messageRecord.FormatString.Length == 0 && messageRecord.FieldCount > 0)
                {
                    messageRecord.FormatString = "1: [1]   2: [2]   3: [3]   4: [4]  5: [5]";
                }
                Console.WriteLine("{0}: {1}", messageType, messageRecord.ToString());
            }
            else
            {
                Console.WriteLine("{0}: (null)", messageType);
            }
            return MessageResult.None;
        }

        [TestMethod]
        public void InstallerMessageResources()
        {
            string message1101 = Installer.GetErrorMessage(1101);
            Console.WriteLine("Message 1101: " + message1101);
            Assert.IsNotNull(message1101);
            Assert.IsTrue(message1101.Contains("file"));
            
            message1101 = Installer.GetErrorMessage(1101, CultureInfo.GetCultureInfo(1033));
            Console.WriteLine("Message 1101: " + message1101);
            Assert.IsNotNull(message1101);
            Assert.IsTrue(message1101.Contains("file"));

            string message2621 = Installer.GetErrorMessage(2621);
            Console.WriteLine("Message 2621: " + message2621);
            Assert.IsNotNull(message2621);
            Assert.IsTrue(message2621.Contains("DLL"));

            string message3002 = Installer.GetErrorMessage(3002);
            Console.WriteLine("Message 3002: " + message3002);
            Assert.IsNotNull(message3002);
            Assert.IsTrue(message3002.Contains("sequencing"));
        }

        [TestMethod]
        public void EnumComponentQualifiers()
        {
            foreach (ComponentInstallation comp in ComponentInstallation.AllComponents)
            {
                bool qualifiers = false;
                foreach (ComponentInstallation.Qualifier qualifier in comp.Qualifiers)
                {
                    if (!qualifiers)
                    {
                        Console.WriteLine(comp.Path);
                        qualifiers = true;
                    }

                    Console.WriteLine("\t{0}: {1}", qualifier.QualifierCode, qualifier.Data);
                }
            }
        }
    }
}
