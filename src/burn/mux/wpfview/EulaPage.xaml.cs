//-------------------------------------------------------------------------------------------------
// <copyright file="EulaPage.xaml.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Setup wizard page that shows the eula
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.IO;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;

    /// <summary>
    /// Interaction logic for the Eula page
    /// </summary>
    public partial class EulaPage : PageBase
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="EulaPage"/> class.
        /// </summary>
        public EulaPage()
            : base()
        {
            InitializeComponent();
            this.LoadEulaText();
        }

        /// <summary>
        /// Loads Eula text into a control
        /// </summary>
        private void LoadEulaText()
        {
            // TODO: add proper localization support
            TextRange range;
            FileStream fileStream;
            string eulaFile = new Uri(System.Reflection.Assembly.GetAssembly(typeof(EulaPage)).CodeBase).LocalPath;
            eulaFile = System.IO.Path.GetDirectoryName(eulaFile);
            string path = System.IO.Path.Combine(eulaFile, "eula.rtf");
          
            if (File.Exists(path))
            {
                range = new TextRange(richTextEula.Document.ContentStart, richTextEula.Document.ContentEnd);
                fileStream = File.OpenRead(path);
                range.Load(fileStream, DataFormats.Rtf);
                fileStream.Close();
            }
        }

        /// <summary>
        /// Event handler to print the Eula
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Routed event</param>
        private void Print_Click(object sender, RoutedEventArgs e)
        {
            PrintDialog printDialog = new PrintDialog();
            if (true == printDialog.ShowDialog())
            {
                printDialog.PrintVisual(richTextEula as Visual, "printing Eula");
            }
        }

        /// <summary>
        /// Event handler to save the Eula
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Routed event</param>
        private void Save_Click(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.SaveFileDialog saveDialog = new System.Windows.Forms.SaveFileDialog();
            saveDialog.Filter = "Rich text|*.rtf";
            saveDialog.FileName = "eula.rtf";

            if (System.Windows.Forms.DialogResult.OK == saveDialog.ShowDialog())
            {
                TextRange range;
                FileStream fileStream;
                range = new TextRange(richTextEula.Document.ContentStart, richTextEula.Document.ContentEnd);
                fileStream = new FileStream(saveDialog.FileName, FileMode.Create);
                range.Save(fileStream, DataFormats.XamlPackage);
                fileStream.Close();
            }
        }
    }
}
