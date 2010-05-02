//-----------------------------------------------------------------------
// <copyright file="EulaPage.xaml.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>eula page</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;

namespace BurnSampleWPFUI
{
    /// <summary>
    /// Interaction logic for EulaPage.xaml
    /// </summary>
    public partial class EulaPage : UserControl
    {
        private BurnSetup m_mainWindow;

        public EulaPage(BurnSetup mainWindow)
        {
            InitializeComponent();
            m_mainWindow = mainWindow;
        }

        public void LoadEulaText()
        {
            TextRange range;
            FileStream fStream;
            string eulaFile = new Uri(System.Reflection.Assembly.GetAssembly(typeof(EulaPage)).CodeBase).LocalPath;
            eulaFile = System.IO.Path.GetDirectoryName(eulaFile);
            eulaFile = System.IO.Path.Combine(eulaFile, "1033"); 
            string path = System.IO.Path.Combine(eulaFile, "eula.rtf");
          
            if (File.Exists(path))
            {
                range = new TextRange(richTextEula.Document.ContentStart, richTextEula.Document.ContentEnd);
                fStream = File.OpenRead(path);
                range.Load(fStream, DataFormats.Rtf);
                fStream.Close();
            }
        }

        public void Proceed()
        {
            m_mainWindow.LoadWelcomePage();
        }

        private void btnDecline_Click(object sender, RoutedEventArgs e)
        {
            m_mainWindow.ShowCancelWindow(ManagedSetupUX.UIPage.Eula);
        }

        private void btnAccept_Click(object sender, RoutedEventArgs e)
        {
            Proceed();
        }

        private void btnPrint_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                PrintDialog dlg = new PrintDialog();
                if (dlg.ShowDialog() == true)
                {
                    dlg.PrintVisual(richTextEula as Visual, "printing Eula");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void btnSave_Click(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.SaveFileDialog sfd = new System.Windows.Forms.SaveFileDialog();
            sfd.Filter = "Rich text|*.rtf";
            sfd.FileName = "eula.rtf";

            if (sfd.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                TextRange range;
                FileStream fStream;
                range = new TextRange(richTextEula.Document.ContentStart, richTextEula.Document.ContentEnd);
                fStream = new FileStream(sfd.FileName, FileMode.Create);
                range.Save(fStream, DataFormats.XamlPackage);
                fStream.Close();
            }
        }
    }
}
