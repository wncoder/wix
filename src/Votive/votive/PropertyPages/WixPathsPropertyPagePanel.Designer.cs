namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio.PropertyPages
{
    partial class WixPathsPropertyPagePanel
    {
        /// <summary> 
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(WixPathsPropertyPagePanel));
            this.referencePathsFoldersSelector = new Microsoft.Tools.WindowsInstallerXml.VisualStudio.Controls.FoldersSelector();
            this.referencePathsGroupBox = new Microsoft.Tools.WindowsInstallerXml.VisualStudio.Controls.WixGroupBox();
            this.includePathsGroupBox = new Microsoft.Tools.WindowsInstallerXml.VisualStudio.Controls.WixGroupBox();
            this.includePathsFolderSelector = new Microsoft.Tools.WindowsInstallerXml.VisualStudio.Controls.FoldersSelector();
            this.mainTableLayoutPanel = new System.Windows.Forms.TableLayoutPanel();
            this.referencePathsGroupBox.SuspendLayout();
            this.includePathsGroupBox.SuspendLayout();
            this.mainTableLayoutPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // referencePathsFoldersSelector
            // 
            resources.ApplyResources(this.referencePathsFoldersSelector, "referencePathsFoldersSelector");
            this.referencePathsFoldersSelector.Name = "referencePathsFoldersSelector";
            this.referencePathsFoldersSelector.Text = "";
            // 
            // referencePathsGroupBox
            // 
            resources.ApplyResources(this.referencePathsGroupBox, "referencePathsGroupBox");
            this.referencePathsGroupBox.Controls.Add(this.referencePathsFoldersSelector);
            this.referencePathsGroupBox.Name = "referencePathsGroupBox";
            // 
            // includePathsGroupBox
            // 
            resources.ApplyResources(this.includePathsGroupBox, "includePathsGroupBox");
            this.includePathsGroupBox.Controls.Add(this.includePathsFolderSelector);
            this.includePathsGroupBox.Name = "includePathsGroupBox";
            // 
            // includePathsFolderSelector
            // 
            resources.ApplyResources(this.includePathsFolderSelector, "includePathsFolderSelector");
            this.includePathsFolderSelector.Name = "includePathsFolderSelector";
            this.includePathsFolderSelector.Text = "";
            // 
            // mainTableLayoutPanel
            // 
            this.mainTableLayoutPanel.Controls.Add(this.includePathsGroupBox, 0, 1);
            this.mainTableLayoutPanel.Controls.Add(this.referencePathsGroupBox, 0, 0);
            resources.ApplyResources(this.mainTableLayoutPanel, "mainTableLayoutPanel");
            this.mainTableLayoutPanel.Name = "mainTableLayoutPanel";
            // 
            // WixPathsPropertyPagePanel
            // 
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.mainTableLayoutPanel);
            this.MinimumSize = new System.Drawing.Size(344, 447);
            this.Name = "WixPathsPropertyPagePanel";
            this.referencePathsGroupBox.ResumeLayout(false);
            this.includePathsGroupBox.ResumeLayout(false);
            this.mainTableLayoutPanel.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private Microsoft.Tools.WindowsInstallerXml.VisualStudio.Controls.FoldersSelector referencePathsFoldersSelector;
        private Microsoft.Tools.WindowsInstallerXml.VisualStudio.Controls.WixGroupBox referencePathsGroupBox;
        private Microsoft.Tools.WindowsInstallerXml.VisualStudio.Controls.WixGroupBox includePathsGroupBox;
        private Microsoft.Tools.WindowsInstallerXml.VisualStudio.Controls.FoldersSelector includePathsFolderSelector;
        private System.Windows.Forms.TableLayoutPanel mainTableLayoutPanel;
    }
}
