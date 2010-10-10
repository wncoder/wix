//-------------------------------------------------------------------------------------------------
// <copyright file="Differ.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// Creates a transform by diffing two outputs.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections;
    using System.Globalization;

    using Microsoft.Tools.WindowsInstallerXml.Msi;

    /// <summary>
    /// Creates a transform by diffing two outputs.
    /// </summary>
    public sealed class Differ : IMessageHandler
    {
        private bool showPedanticMessages;
        private bool suppressKeepingSpecialRows;
        private bool preserveUnchangedRows;
        private const char sectionDelimiter = '/';

        /// <summary>
        /// Instantiates a new Differ class.
        /// </summary>
        public Differ()
        {
        }

        /// <summary>
        /// Event for messages.
        /// </summary>
        public event MessageEventHandler Message;

        /// <summary>
        /// Gets or sets the option to show pedantic messages.
        /// </summary>
        /// <value>The option to show pedantic messages.</value>
        public bool ShowPedanticMessages
        {
            get { return this.showPedanticMessages; }
            set { this.showPedanticMessages = value; }
        }

        /// <summary>
        /// Gets or sets the option to suppress keeping special rows.
        /// </summary>
        /// <value>The option to suppress keeping special rows.</value>
        public bool SuppressKeepingSpecialRows
        {
            get { return this.suppressKeepingSpecialRows; }
            set { this.suppressKeepingSpecialRows = value; }
        }

        /// <summary>
        /// Gets or sets the flag to determine if all rows, even unchanged ones will be persisted in the output.
        /// </summary>
        /// <value>The option to keep all rows including unchanged rows.</value>
        public bool PreserveUnchangedRows
        {
            get { return this.preserveUnchangedRows; }
            set { this.preserveUnchangedRows = value; }
        }

        /// <summary>
        /// Creates a transform by diffing two outputs.
        /// </summary>
        /// <param name="targetOutput">The target output.</param>
        /// <param name="updatedOutput">The updated output.</param>
        /// <returns>The transform.</returns>
        public Output Diff(Output targetOutput, Output updatedOutput)
        {
            return Diff(targetOutput, updatedOutput, 0);
        }

        /// <summary>
        /// Creates a transform by diffing two outputs.
        /// </summary>
        /// <param name="targetOutput">The target output.</param>
        /// <param name="updatedOutput">The updated output.</param>
        /// <param name="validationFlags"></param>
        /// <returns>The transform.</returns>
        public Output Diff(Output targetOutput, Output updatedOutput, TransformFlags validationFlags)
        {
            Output transform = new Output(null);
            transform.Type = OutputType.Transform;
            transform.Codepage = updatedOutput.Codepage;

            string targetSummaryInfoCodepage = null;
            string targetPlatformAndLanguage = null;
            string targetProductCode = null;
            string targetProductVersion = null;
            string targetUpgradeCode = null;
            string targetMinimumVersion = null;
            string updatedSummaryInfoCodepage = null;
            string updatedPlatformAndLanguage = null;
            string updatedProductCode = null;
            string updatedProductVersion = null;
            string updatedMinimumVersion = null;

            // compare the codepages
            if (targetOutput.Codepage != updatedOutput.Codepage && 0 == (TransformFlags.ErrorChangeCodePage & validationFlags))
            {
                this.OnMessage(WixErrors.OutputCodepageMismatch(targetOutput.SourceLineNumbers, targetOutput.Codepage, updatedOutput.Codepage));
                if (null != updatedOutput.SourceLineNumbers)
                {
                    this.OnMessage(WixErrors.OutputCodepageMismatch2(updatedOutput.SourceLineNumbers));
                }
            }

            // compare the output types
            if (targetOutput.Type != updatedOutput.Type)
            {
                throw new WixException(WixErrors.OutputTypeMismatch(targetOutput.SourceLineNumbers, targetOutput.Type.ToString(), updatedOutput.Type.ToString()));
            }

            // compare the contents of the tables
            foreach (Table targetTable in targetOutput.Tables)
            {
                Table updatedTable = updatedOutput.Tables[targetTable.Name];

                // dropped tables
                if (null == updatedTable)
                {
                    Table droppedTable = transform.Tables.EnsureTable(null, targetTable.Definition);
                    droppedTable.Operation = TableOperation.Drop;
                }
                else // possibly modified tables
                {
                    SortedList updatedPrimaryKeys = new SortedList();
                    SortedList targetPrimaryKeys = new SortedList();

                    // compare the table definitions
                    if (0 != targetTable.Definition.CompareTo(updatedTable.Definition))
                    {
                        // continue to the next table; may be more mismatches
                        this.OnMessage(WixErrors.DatabaseSchemaMismatch(targetOutput.SourceLineNumbers, targetTable.Name));
                        continue;
                    }

                    // index the target rows
                    foreach (Row row in targetTable.Rows)
                    {
                        this.AddIndexedRow(targetPrimaryKeys, row);

                        if ("Property" == targetTable.Name)
                        {
                            if ("ProductCode" == (string)row[0])
                            {
                                targetProductCode = (string)row[1];
                                if ("*" == targetProductCode)
                                {
                                    this.OnMessage(WixErrors.ProductCodeInvalidForTransform(row.SourceLineNumbers));
                                }
                            }
                            else if ("ProductVersion" == (string)row[0])
                            {
                                targetProductVersion = (string)row[1];
                            }
                            else if ("UpgradeCode" == (string)row[0])
                            {
                                targetUpgradeCode = (string)row[1];
                            }
                        }
                        else if ("_SummaryInformation" == targetTable.Name)
                        {
                            if (1 == (int)row[0]) // PID_CODEPAGE
                            {
                                targetSummaryInfoCodepage = (string)row[1];
                            }
                            else if (7 == (int)row[0]) // PID_TEMPLATE
                            {
                                targetPlatformAndLanguage = (string)row[1];
                            }
                            else if (14 == (int)row[0]) // PID_PAGECOUNT
                            {
                                targetMinimumVersion = (string)row[1];
                            }
                        }
                    }

                    // index the updated rows
                    foreach (Row row in updatedTable.Rows)
                    {
                        this.AddIndexedRow(updatedPrimaryKeys, row);

                        if ("Property" == updatedTable.Name)
                        {
                            if ("ProductCode" == (string)row[0])
                            {
                                updatedProductCode = (string)row[1];
                                if ("*" == updatedProductCode)
                                {
                                    this.OnMessage(WixErrors.ProductCodeInvalidForTransform(row.SourceLineNumbers));
                                }
                            }
                            else if ("ProductVersion" == (string)row[0])
                            {
                                updatedProductVersion = (string)row[1];
                            }
                        }
                        else if ("_SummaryInformation" == updatedTable.Name)
                        {
                            if (1 == (int)row[0]) // PID_CODEPAGE
                            {
                                updatedSummaryInfoCodepage = (string)row[1];
                            }
                            else if (7 == (int)row[0]) // PID_TEMPLATE
                            {
                                updatedPlatformAndLanguage = (string)row[1];
                            }
                            else if (14 == (int)row[0]) // PID_PAGECOUNT
                            {
                                updatedMinimumVersion = (string)row[1];
                            }
                        }
                    }

                    // diff the target and updated rows
                    foreach (DictionaryEntry targetPrimaryKeyEntry in targetPrimaryKeys)
                    {
                        string targetPrimaryKey = (string)targetPrimaryKeyEntry.Key;
                        Row targetRow = (Row)targetPrimaryKeyEntry.Value;
                        Row updatedRow = (Row)updatedPrimaryKeys[targetPrimaryKey];

                        if (null == updatedRow) // deleted row
                        {
                            Table modifiedTable = transform.EnsureTable(targetTable.Definition);
                            targetRow.Operation = RowOperation.Delete;
                            targetRow.SectionId = targetRow.SectionId + sectionDelimiter;
                            modifiedTable.Rows.Add(targetRow);
                        }
                        else // possibly modified
                        {
                            updatedRow.Operation = RowOperation.None;
                            if (!this.suppressKeepingSpecialRows && "_SummaryInformation" == targetTable.Name)
                            {
                                // ignore rows that shouldn't be in a transform
                                if (Enum.IsDefined(typeof(SummaryInformation.Transform), (int)updatedRow[0]))
                                {
                                    Table table = transform.EnsureTable(updatedTable.Definition);
                                    updatedRow.SectionId = targetRow.SectionId + sectionDelimiter + updatedRow.SectionId;
                                    table.Rows.Add(updatedRow);
                                }
                            }
                            else
                            {
                                bool keepRow = false;

                                if (this.preserveUnchangedRows)
                                {
                                    keepRow = true;
                                }

                                for (int i = 0; i < updatedRow.Fields.Length; i++)
                                {
                                    ColumnDefinition columnDefinition = updatedRow.Fields[i].Column;

                                    if (!columnDefinition.IsPrimaryKey)
                                    {
                                        bool modified = false;

                                        if (i >= targetRow.Fields.Length)
                                        {
                                            columnDefinition.Added = true;
                                            modified = true;
                                        }
                                        else if (ColumnType.Number == columnDefinition.Type && !columnDefinition.IsLocalizable)
                                        {
                                            if (null == targetRow[i] ^ null == updatedRow[i])
                                            {
                                                modified = true;
                                            }
                                            else if (null != targetRow[i] && null != updatedRow[i])
                                            {
                                                modified = ((int)targetRow[i] != (int)updatedRow[i]);
                                            }
                                        }
                                        else if (ColumnType.Preserved == columnDefinition.Type)
                                        {
                                            updatedRow.Fields[i].PreviousData = (string)targetRow.Fields[i].Data;

                                            // keep rows containing preserved fields so the historical data is available to the binder
                                            keepRow = !this.suppressKeepingSpecialRows;
                                        }
                                        else if (ColumnType.Object == columnDefinition.Type)
                                        {
                                            ObjectField targetObjectField = (ObjectField)targetRow.Fields[i];
                                            ObjectField updatedObjectField = (ObjectField)updatedRow.Fields[i];

                                            updatedObjectField.PreviousCabinetFileId = targetObjectField.CabinetFileId;
                                            updatedObjectField.PreviousBaseUri = targetObjectField.BaseUri;

                                            if ((string)targetObjectField.Data != (string)updatedObjectField.Data)
                                            {
                                                updatedObjectField.PreviousData = (string)targetObjectField.Data;
                                            }

                                            // keep rows containing object fields so the files can be compared in the binder
                                            keepRow = !this.suppressKeepingSpecialRows;
                                        }
                                        else
                                        {
                                            modified = ((string)targetRow[i] != (string)updatedRow[i]);
                                        }

                                        if (modified)
                                        {
                                            updatedRow.Fields[i].Modified = true;
                                            updatedRow.Operation = RowOperation.Modify;
                                            keepRow = true;
                                        }
                                    }
                                }

                                if (keepRow)
                                {
                                    Table modifiedTable = transform.EnsureTable(updatedTable.Definition);
                                    updatedRow.SectionId = targetRow.SectionId + sectionDelimiter + updatedRow.SectionId;
                                    modifiedTable.Rows.Add(updatedRow);
                                }
                            }
                        }
                    }

                    // find the inserted rows
                    foreach (DictionaryEntry updatedPrimaryKeyEntry in updatedPrimaryKeys)
                    {
                        string updatedPrimaryKey = (string)updatedPrimaryKeyEntry.Key;

                        if (!targetPrimaryKeys.Contains(updatedPrimaryKey))
                        {
                            Row updatedRow = (Row)updatedPrimaryKeyEntry.Value;

                            Table modifiedTable = transform.EnsureTable(updatedTable.Definition);
                            updatedRow.Operation = RowOperation.Add;
                            updatedRow.SectionId = sectionDelimiter + updatedRow.SectionId;
                            modifiedTable.Rows.Add(updatedRow);
                        }
                    }
                }
            }

            // added tables
            foreach (Table updatedTable in updatedOutput.Tables)
            {
                if (null == targetOutput.Tables[updatedTable.Name])
                {
                    Table addedTable = transform.Tables.EnsureTable(null, updatedTable.Definition);
                    addedTable.Operation = TableOperation.Add;

                    foreach (Row updatedRow in updatedTable.Rows)
                    {
                        updatedRow.Operation = RowOperation.Add;
                        updatedRow.SectionId = sectionDelimiter + updatedRow.SectionId;
                        addedTable.Rows.Add(updatedRow);
                    }
                }
            }
            
            // set summary information properties
            if (!this.suppressKeepingSpecialRows)
            {
                // calculate the minimum version of MSI required to process the transform
                int targetMin;
                int updatedMin;
                int minimumVersion = 100;

                if (Int32.TryParse(targetMinimumVersion, out targetMin) && Int32.TryParse(updatedMinimumVersion, out updatedMin))
                {
                    minimumVersion = Math.Max(targetMin, updatedMin);
                }

                Table summaryInfoTable = transform.Tables["_SummaryInformation"];
                Hashtable summaryRows = new Hashtable();
                foreach (Row row in summaryInfoTable.Rows)
                {
                    summaryRows[row[0]] = row;

                    if ((int)SummaryInformation.Transform.CodePage == (int)row[0])
                    {
                        row.Fields[1].Data = updatedSummaryInfoCodepage;
                        row.Fields[1].PreviousData = targetSummaryInfoCodepage;
                    }
                    else if ((int)SummaryInformation.Transform.TargetPlatformAndLanguage == (int)row[0])
                    {
                        row[1] = targetPlatformAndLanguage;
                    }
                    else if ((int)SummaryInformation.Transform.UpdatedPlatformAndLanguage == (int)row[0])
                    {
                        row[1] = updatedPlatformAndLanguage;
                    }
                    else if ((int)SummaryInformation.Transform.ProductCodes == (int)row[0])
                    {
                        row[1] = String.Concat(targetProductCode, targetProductVersion, ';', updatedProductCode, updatedProductVersion, ';', targetUpgradeCode);
                    }
                    else if ((int)SummaryInformation.Transform.InstallerRequirement == (int)row[0])
                    {
                        row[1] = minimumVersion.ToString(CultureInfo.InvariantCulture);
                    }
                    else if ((int)SummaryInformation.Transform.Security == (int)row[0])
                    {
                        row[1] = "4";
                    }
                }

                if (!summaryRows.Contains((int)SummaryInformation.Transform.TargetPlatformAndLanguage))
                {
                    Row summaryRow = summaryInfoTable.CreateRow(null);
                    summaryRow[0] = (int)SummaryInformation.Transform.TargetPlatformAndLanguage;
                    summaryRow[1] = targetPlatformAndLanguage;
                }

                if (!summaryRows.Contains((int)SummaryInformation.Transform.UpdatedPlatformAndLanguage))
                {
                    Row summaryRow = summaryInfoTable.CreateRow(null);
                    summaryRow[0] = (int)SummaryInformation.Transform.UpdatedPlatformAndLanguage;
                    summaryRow[1] = updatedPlatformAndLanguage;
                }

                if (!summaryRows.Contains((int)SummaryInformation.Transform.ValidationFlags))
                {
                    Row summaryRow = summaryInfoTable.CreateRow(null);
                    summaryRow[0] = (int)SummaryInformation.Transform.ValidationFlags;
                    summaryRow[1] = ((int)validationFlags).ToString(CultureInfo.InvariantCulture);
                }

                if (!summaryRows.Contains((int)SummaryInformation.Transform.InstallerRequirement))
                {
                    Row summaryRow = summaryInfoTable.CreateRow(null);
                    summaryRow[0] = (int)SummaryInformation.Transform.InstallerRequirement;
                    summaryRow[1] = minimumVersion.ToString(CultureInfo.InvariantCulture);
                }

                if (!summaryRows.Contains((int)SummaryInformation.Transform.Security))
                {
                    Row summaryRow = summaryInfoTable.CreateRow(null);
                    summaryRow[0] = (int)SummaryInformation.Transform.Security;
                    summaryRow[1] = "4";
                }
            }

            return transform;
        }

        /// <summary>
        /// Sends a message to the message delegate if there is one.
        /// </summary>
        /// <param name="mea">Message event arguments.</param>
        public void OnMessage(MessageEventArgs e)
        {
            WixErrorEventArgs errorEventArgs = e as WixErrorEventArgs;

            if (null != this.Message)
            {
                this.Message(this, e);
            }
            else if (null != errorEventArgs)
            {
                throw new WixException(errorEventArgs);
            }
        }

        /// <summary>
        /// Add a row to the <paramref name="index"/> using the primary key.
        /// </summary>
        /// <param name="index">The indexed rows.</param>
        /// <param name="row">The row to index.</param>
        private void AddIndexedRow(IDictionary index, Row row)
        {
            string primaryKey = row.GetPrimaryKey('/');
            if (null != primaryKey)
            {
                // Overriding WixActionRows have a primary key defined and take precedence in the index.
                if (row is WixActionRow)
                {
                    WixActionRow currentRow = (WixActionRow)row;
                    if (index.Contains(primaryKey))
                    {
                        // If the current row is not overridable, see if the indexed row is.
                        if (!currentRow.Overridable)
                        {
                            WixActionRow indexedRow = index[primaryKey] as WixActionRow;
                            if (null != indexedRow && indexedRow.Overridable)
                            {
                                // The indexed key is overridable and should be replaced
                                // (not removed and re-added which results in two Array.Copy
                                // operations for SortedList, or may be re-hashing in other
                                // implementations of IDictionary).
                                index[primaryKey] = currentRow;
                            }
                        }

                        // If we got this far, the row does not need to be indexed.
                        return;
                    }
                }

                // Nothing else should be added more than once.
                if (!index.Contains(primaryKey))
                {
                    index.Add(primaryKey, row);
                }
                else if (this.showPedanticMessages)
                {
                    this.OnMessage(WixWarnings.DuplicatePrimaryKey(row.SourceLineNumbers, primaryKey, row.Table.Name));
                }
            }
            else // use the string representation of the row as its primary key (it may not be unique)
            {
                // this is provided for compatibility with unreal tables with no primary key
                // all real tables must specify at least one column as the primary key
                primaryKey = row.ToString();
                index[primaryKey] = row;
            }
        }
    }
}
