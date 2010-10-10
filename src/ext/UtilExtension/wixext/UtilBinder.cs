//-------------------------------------------------------------------------------------------------
// <copyright file="UtilBinder.cs" company="Microsoft">
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
// The binder for the Windows Installer XML Toolset Utility Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Globalization;
    using System.Text;

    /// <summary>
    /// The binder for the Windows Installer XML Toolset Utility Extension.
    /// </summary>
    public sealed class UtilBinder : BinderExtension
    {
        /// <summary>
        /// Instantiate a new UtilBinder.
        /// </summary>
        public UtilBinder()
        {
        }

        // TODO: When WixSearch is supported in Product, etc, we may need to call
        // ReorderWixSearch() from each of those initializers. 

        // TODO: A general-purpose "reorder this table given these constraints"
        // mechanism may end up being helpful. This could be declaratively stated
        // in the table definitions, or exposed from the core Wix.dll and called
        // as-needed by any extensions.

        /// <summary>
        /// Called before bundle binding occurs.
        /// </summary>
        public override void BundleInitialize(Output bundle)
        {
            this.ReorderWixSearch(bundle);
        }

        /// <summary>
        /// Reorders Any WixSearch items.
        /// </summary>
        /// <param name="output">Output containing the tables to process.</param>
        private void ReorderWixSearch(Output output)
        {
            Table wixSearchTable = output.Tables["WixSearch"];
            if (null == wixSearchTable || wixSearchTable.Rows.Count == 0)
            {
                // nothing to do!
                return;
            }

            RowDictionary rowDictionary = new RowDictionary();
            foreach (Row row in wixSearchTable.Rows)
            {
                rowDictionary.AddRow(row);
            }

            Constraints constraints = new Constraints();
            Table wixSearchRelationTable = output.Tables["WixSearchRelation"];
            if (null != wixSearchRelationTable && wixSearchRelationTable.Rows.Count > 0)
            {
                // add relational info to our data...
                foreach (Row row in wixSearchRelationTable.Rows)
                {
                    constraints.AddConstraint((string)row[0], (string)row[1]);
                }
            }

            this.FindCircularReference(constraints);

            if (this.Core.EncounteredError)
            {
                return;
            }

            this.FlattenDependentReferences(constraints);

            // Reorder by ID, then shuffle to fix up any dependencies.
            // We can do this in a single operation with a smart comparer..
            List<string> sortedIds = new List<string>(rowDictionary.Keys);
            ConstraintComparer comparer = new ConstraintComparer(constraints);
            sortedIds.Sort(comparer);

            // Now, re-write the table with the searches in order...
            wixSearchTable.Rows.Clear();
            foreach (string id in sortedIds)
            {
                wixSearchTable.Rows.Add(rowDictionary[id]);
            }
        }

        /// <summary>
        /// A dictionary of Row items, indexed by their first column.
        /// </summary>
        private class RowDictionary : Dictionary<string, Row>
        {
            public void AddRow(Row row)
            {
                this.Add((string)row[0], row);
            }

            // TODO: Hide other Add methods?
        }

        /// <summary>
        /// A dictionary of constraints, mapping an id to a list of ids.
        /// </summary>
        private class Constraints : Dictionary<string, List<string>>
        {
            public void AddConstraint(string id, string afterId)
            {
                if (!this.ContainsKey(id))
                {
                    this.Add(id, new List<string>());
                }

                // TODO: Show warning if a constraint is seen twice?
                if (!this[id].Contains(afterId))
                {
                    this[id].Add(afterId);
                }
            }

            // TODO: Hide other Add methods?
        }

        /// <summary>
        /// A string Comparer that uses a Constaints dictionary to order items.
        /// </summary>
        private class ConstraintComparer : Comparer<string>
        {
            public ConstraintComparer(Constraints constraints)
            {
                this.constraints = constraints;
            }

            private Constraints constraints;

            public override int Compare(string x, string y)
            {
                // check the constraints first..
                List<string> afterIds;
                if (this.constraints.TryGetValue(x, out afterIds))
                {
                    if (afterIds.Contains(y))
                    {
                        return 1;
                    }
                }

                if (this.constraints.TryGetValue(y, out afterIds))
                {
                    if (afterIds.Contains(x))
                    {
                        return -1;
                    }
                }


                // If all else fails, just compare the ids...
                return string.Compare(x, y, StringComparison.Ordinal);
            }
        }

        /// <summary>
        /// Finds circular references in the constraints.
        /// </summary>
        /// <param name="constraints">Constraints to check.</param>
        /// <remarks>This is not particularly performant, but it works.</remarks>
        private void FindCircularReference(Constraints constraints)
        {
            foreach (string id in constraints.Keys)
            {
                List<string> seenIds = new List<string>();
                string chain = null;
                if (FindCircularReference(constraints, id, id, seenIds, out chain))
                {
                    // We will show a separate message for every ID that's in
                    // the loop. We could bail after the first one, but then
                    // we wouldn't catch disjoint loops in a single run.
                    this.Core.OnMessage(UtilErrors.CircularSearchReference(chain));
                }
            }
        }

        /// <summary>
        /// Recursive function that finds circular references in the constraints.
        /// </summary>
        /// <param name="constraints">Constraints to check.</param>
        /// <param name="checkId">The identifier currently being looking for. (Fixed across a given run.)</param>
        /// <param name="currentId">The idenifier curently being tested.</param>
        /// <param name="seenIds">A list of identifiers seen, to ensure each identifier is only expanded once.</param>
        /// <param name="chain">If a circular reference is found, will contain the chain of references.</param>
        /// <returns>True if a circular reference is found, false otherwise.</returns>
        private bool FindCircularReference(Constraints constraints, string checkId, string currentId, List<string> seenIds, out string chain)
        {
            chain = null;
            List<string> afterList = null;
            if (constraints.TryGetValue(currentId, out afterList))
            {
                foreach (string afterId in afterList)
                {
                    if (afterId == checkId)
                    {
                        chain = String.Format(CultureInfo.InvariantCulture, "{0} -> {1}", currentId, afterId);
                        return true;
                    }

                    if (!seenIds.Contains(afterId))
                    {
                        seenIds.Add(afterId);
                        if (FindCircularReference(constraints, checkId, afterId, seenIds, out chain))
                        {
                            chain = String.Format(CultureInfo.InvariantCulture, "{0} -> {1}", currentId, chain);
                            return true;
                        }
                    }
                }
            }

            return false;
        }

        /// <summary>
        /// Flattens any dependency chains to simplify reordering.
        /// </summary>
        /// <param name="constraints"></param>
        private void FlattenDependentReferences(Constraints constraints)
        {
            foreach (string id in constraints.Keys)
            {
                List<string> flattenedIds = new List<string>();
                AddDependentReferences(constraints, id, flattenedIds);
                List<string> constraintList = constraints[id];
                foreach (string flattenedId in flattenedIds)
                {
                    if (!constraintList.Contains(flattenedId))
                    {
                        constraintList.Add(flattenedId);
                    }
                }
            }
        }

        /// <summary>
        /// Adds dependent references to a list.
        /// </summary>
        /// <param name="constraints"></param>
        /// <param name="currentId"></param>
        /// <param name="seenIds"></param>
        private void AddDependentReferences(Constraints constraints, string currentId, List<string> seenIds)
        {
            List<string> afterList = null;
            if (constraints.TryGetValue(currentId, out afterList))
            {
                foreach (string afterId in afterList)
                {
                    if (!seenIds.Contains(afterId))
                    {
                        seenIds.Add(afterId);
                        AddDependentReferences(constraints, afterId, seenIds);
                    }
                }
            }
        }


    }
}
