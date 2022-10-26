using DocumentFormat.OpenXml.CustomProperties;
using DocumentFormat.OpenXml.ExtendedProperties;
using DocumentFormat.OpenXml.Packaging;
using DocumentFormat.OpenXml.VariantTypes;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
namespace OfficeTagLib
{
    internal class Log
    {
        public static void WriteLine(string strMsg)
        {
            System.Diagnostics.Trace.WriteLine("[OfficeTagLib]------" + strMsg);
            Console.WriteLine("[OfficeTagLib]------" + strMsg);
        }
        public static void WriteLine(Exception ex)
        {
            System.Diagnostics.Trace.WriteLine("[OfficeTagLib]------Exception:" + ex.Message + " StackTrace" + ex.StackTrace);
            Console.WriteLine("[OfficeTagLib]------Exception:" + ex.Message + " StackTrace" + ex.StackTrace);
        }
    }
    public class TagOpera
    {
        private const string STR_SUMMARY_TAG_TITLE_NAME = "Title";
        private const string STR_SUMMARY_TAG_SUBJECT_NAME = "Subject";
        private const string STR_SUMMARY_TAG_AUTHOR_NAME = "Author";
        private const string STR_SUMMARY_TAG_CATEGORY_NAME = "Category";
        private const string STR_SUMMARY_TAG_KEYWORDS_NAME = "Keywords";
        private const string STR_SUMMARY_TAG_COMMENTS_NAME = "Comments";

        private const string STR_SUMMARY_TAG_MANAGE_NAME = "Manager";
        private const string STR_SUMMARY_TAG_COMPANY_NAME = "Company";
        private const string STR_SUMMARY_TAG_HYPERLINKBASE_NAME = "HyperlinkBase";
        private const string STR_SUMMARY_TAG_TEMPLATE_NAME = "Template";

        public TagOpera()
        {
            m_DicTagCollection = new Dictionary<string, List<string>>();
        }

        private Dictionary<string, List<string>> m_DicTagCollection = null;
        private string m_StrFilePath = null;
        private Dictionary<string, List<string>> TagCollection
        {
            get
            {
                return m_DicTagCollection;
            }
        }
        private string FilePath
        {
            set
            {
                m_StrFilePath = value;
            }
            get
            {
                return m_StrFilePath;
            }
        }

        public void SetOfficeFilePath(string strFilePath)
        {
            FilePath = strFilePath;
        }
        public void SetTag(string strKey, string strValue)
        {
            if (m_DicTagCollection.ContainsKey(strKey))
            {
                if (!string.IsNullOrEmpty(strValue))
                {
                    m_DicTagCollection[strKey].Add(strValue);
                }
            }
            else
            {
                List<string> lisValue = new List<string>();
                if (!string.IsNullOrEmpty(strValue))
                {
                    lisValue.Add(strValue);
                }
                m_DicTagCollection.Add(strKey, lisValue);

            }
        }
        public void ClearTag()
        {
            TagCollection.Clear();
        }
        public int GetTagKeyCount()
        {
            return m_DicTagCollection.Keys.Count;
        }
        public string GetTagKey(int index)
        {
            return m_DicTagCollection.Keys.ToArray<string>()[index];
        }
        public int GetTagValueCount(string strKey)
        {
            return m_DicTagCollection[strKey].Count;
        }
        public string GetTagValue(string strKey, int index)
        {
            return m_DicTagCollection[strKey][index];
        }
        public bool ExecuteAdd(bool bOverWrite)
        {
            Log.WriteLine("ExecuteAdd Start:" + FilePath);
            bool bresult = false;
            try
            {
                if (File.Exists(FilePath))
                {
                    if (new FileInfo(FilePath).Length > 0)
                    {
                        FileType fileType = GetFileType(FilePath);
                        switch (fileType)
                        {
                            case FileType.WORD:
                                {
                                    Log.WriteLine("File Type is Word");
                                    var document = WordprocessingDocument.Open(FilePath, true);
                                    var customProps = document.CustomFilePropertiesPart;
                                    if (customProps == null)
                                    {
                                        customProps = document.AddCustomFilePropertiesPart();
                                        customProps.Properties = new DocumentFormat.OpenXml.CustomProperties.Properties();
                                    }
                                    Log.WriteLine("Get Custom Props");
                                    DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                    AddPropertiesFormTagCollection(props, TagCollection, bOverWrite);
                                    props.Save();
                                    document.Close();
                                    bresult = true;
                                }
                                break;
                            case FileType.EXCEL:
                                {
                                    Log.WriteLine("File Type is Excel");
                                    var document = SpreadsheetDocument.Open(FilePath, true);
                                    var customProps = document.CustomFilePropertiesPart;
                                    if (customProps == null)
                                    {
                                        customProps = document.AddCustomFilePropertiesPart();
                                        customProps.Properties = new DocumentFormat.OpenXml.CustomProperties.Properties();
                                    }
                                    DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                    AddPropertiesFormTagCollection(props, TagCollection, bOverWrite);
                                    props.Save();
                                    document.Close();
                                    bresult = true;
                                }
                                break;
                            case FileType.POWERPOINT:
                                {
                                    var document = PresentationDocument.Open(FilePath, true);
                                    var customProps = document.CustomFilePropertiesPart;
                                    if (customProps == null)
                                    {
                                        customProps = document.AddCustomFilePropertiesPart();
                                        customProps.Properties = new DocumentFormat.OpenXml.CustomProperties.Properties();
                                    }
                                    DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                    AddPropertiesFormTagCollection(props, TagCollection, bOverWrite);
                                    props.Save();
                                    document.Close();
                                    bresult = true;
                                }
                                break;
                            case FileType.OTHER:
                                {
                                    Log.WriteLine("Type is Other");
                                    bresult = false;
                                }
                                break;
                        }
                    }
                    else
                    {
                        Log.WriteLine("FILE IS SIZE IS ZERO , BUT RETURN TRUE");
                        bresult = true;
                    }
                }
                else
                {
                    Log.WriteLine("FILE IS NOT EXITES:" + FilePath);
                }
            }
            catch (Exception ex)
            {
                Log.WriteLine(ex);
                bresult = false; ;
            }
            finally
            {
                Log.WriteLine("ExecuteAdd End result is :" + bresult);
            }
            return bresult;
        }
        public bool ExecuteRemove()
        {
            Log.WriteLine("ExecuteRemove Start:" + FilePath);
            bool bresult = false;
            try
            {
                if (File.Exists(FilePath))
                {
                    if (new FileInfo(FilePath).Length > 0)
                    {
                        FileType fileType = GetFileType(FilePath);
                        switch (fileType)
                        {
                            case FileType.WORD:
                                {
                                    Log.WriteLine("File Type is Word");
                                    var document = WordprocessingDocument.Open(FilePath, true);
                                    var customProps = document.CustomFilePropertiesPart;
                                    if (customProps != null)
                                    {
                                        Log.WriteLine("Get Custom Props");
                                        DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                        RemovePropertiesByTagCollection(props, TagCollection);
                                        props.Save();
                                    }
                                    document.Close();
                                    bresult = true;
                                }
                                break;
                            case FileType.EXCEL:
                                {
                                    Log.WriteLine("File Type is Excel");
                                    var document = SpreadsheetDocument.Open(FilePath, true);
                                    var customProps = document.CustomFilePropertiesPart;
                                    if (customProps != null)
                                    {
                                        DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                        RemovePropertiesByTagCollection(props, TagCollection);
                                        props.Save();
                                    }
                                    document.Close();
                                    bresult = true;
                                }
                                break;
                            case FileType.POWERPOINT:
                                {
                                    var document = PresentationDocument.Open(FilePath, true);
                                    var customProps = document.CustomFilePropertiesPart;
                                    if (customProps != null)
                                    {
                                        DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                        RemovePropertiesByTagCollection(props, TagCollection);
                                        props.Save();
                                    }
                                    document.Close();
                                    bresult = true;
                                }
                                break;
                            case FileType.OTHER:
                                {
                                    Log.WriteLine("Type is Other");
                                    bresult = false;
                                }
                                break;
                        }
                    }
                    else
                    {
                        Log.WriteLine("FILE IS SIZE IS ZERO , BUT RETURN TRUE");
                        bresult = true;
                    }
                }
                else
                {
                    Log.WriteLine("FILE IS NOT EXITES:" + FilePath);
                }
            }
            catch (Exception ex)
            {
                Log.WriteLine(ex);
                bresult = false;
            }
            finally
            {
                Log.WriteLine("ExecuteRemove End result is :" + bresult);
            }
            return bresult;
        }
        public bool ExecuteRemoveAll()
        {
            Log.WriteLine("ExecuteRemoveAll Start:" + FilePath);
            bool bresult = false;
            try
            {
                if (File.Exists(FilePath))
                {
                    Log.WriteLine("ExecuteRemoveAll Start:" + FilePath);
                    if (new FileInfo(FilePath).Length > 0)
                    {
                        FileType fileType = GetFileType(FilePath);
                        switch (fileType)
                        {
                            case FileType.WORD:
                                {
                                    Log.WriteLine("File Type is Word");
                                    var document = WordprocessingDocument.Open(FilePath, true);
                                    var customProps = document.CustomFilePropertiesPart;
                                    if (customProps != null)
                                    {
                                        DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                        props.RemoveAllChildren();
                                        props.Save();
                                    }
                                    document.Close();
                                    bresult = true;
                                }
                                break;
                            case FileType.EXCEL:
                                {
                                    Log.WriteLine("File Type is Excel");
                                    var document = SpreadsheetDocument.Open(FilePath, true);
                                    var customProps = document.CustomFilePropertiesPart;
                                    if (customProps != null)
                                    {
                                        DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                        props.RemoveAllChildren();
                                        props.Save();
                                    }
                                    document.Close();
                                    bresult = true;
                                }
                                break;
                            case FileType.POWERPOINT:
                                {
                                    var document = PresentationDocument.Open(FilePath, true);
                                    var customProps = document.CustomFilePropertiesPart;
                                    if (customProps != null)
                                    {
                                        DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                        props.RemoveAllChildren();
                                        props.Save();
                                    }
                                    document.Close();
                                    bresult = true;
                                }
                                break;
                            case FileType.OTHER:
                                {
                                    Log.WriteLine("Type is Other");
                                    bresult = false;
                                }
                                break;
                        }
                    }
                    else
                    {
                        Log.WriteLine("FILE IS SIZE IS ZERO , BUT RETURN TRUE");
                        bresult = true;
                    }
                }
                else
                {
                    Log.WriteLine("FILE IS NOT EXITES:" + FilePath);
                }
            }
            catch (Exception ex)
            {
                Log.WriteLine(ex);
                bresult = false;
            }
            finally
            {
                Log.WriteLine("ExecuteRemoveAll End result is :" + bresult);
            }
            return bresult;
        }
        public bool ExecuteUpdata()
        {
            Log.WriteLine("ExecuteUpdata Start:" + FilePath);
            bool bresult = false;
            try
            {
                if (File.Exists(FilePath))
                {
                    if (new FileInfo(FilePath).Length > 0)
                    {
                        FileType fileType = GetFileType(FilePath);
                        switch (fileType)
                        {
                            case FileType.WORD:
                                {
                                    Log.WriteLine("File Type is Word");
                                    var document = WordprocessingDocument.Open(FilePath, true);
                                    var customProps = document.CustomFilePropertiesPart;
                                    if (customProps != null)
                                    {
                                        Log.WriteLine("Get Custom Props");
                                        DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                        UpdatePropertiesByTagCollection(props, TagCollection);
                                        props.Save();
                                    }
                                    document.Close();
                                    bresult = true;
                                }
                                break;
                            case FileType.EXCEL:
                                {
                                    Log.WriteLine("File Type is Excel");
                                    var document = SpreadsheetDocument.Open(FilePath, true);
                                    var customProps = document.CustomFilePropertiesPart;
                                    if (customProps != null)
                                    {
                                        DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                        UpdatePropertiesByTagCollection(props, TagCollection);
                                        props.Save();
                                    }
                                    document.Close();
                                    bresult = true;
                                }
                                break;
                            case FileType.POWERPOINT:
                                {
                                    var document = PresentationDocument.Open(FilePath, true);
                                    var customProps = document.CustomFilePropertiesPart;
                                    if (customProps != null)
                                    {
                                        DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                        UpdatePropertiesByTagCollection(props, TagCollection);
                                        props.Save();
                                    }
                                    document.Close();
                                    bresult = true;
                                }
                                break;
                            case FileType.OTHER:
                                {
                                    Log.WriteLine("Type is Other");
                                    bresult = false;
                                }
                                break;
                        }
                    }
                    else
                    {
                        Log.WriteLine("FILE IS SIZE IS ZERO , BUT RETURN TRUE");
                        bresult = true;
                    }
                }
                else
                {
                    Log.WriteLine("FILE IS NOT EXITES:" + FilePath);
                }
            }
            catch (Exception ex)
            {
                Log.WriteLine(ex);
                bresult = false;
            }
            finally
            {
                Log.WriteLine("ExecuteUpdata End result is :" + bresult);
            }
            return bresult;
        }
        public bool ExecuteReadCustomTag()
        {
            Log.WriteLine("ExecuteReadCustomTag Start:" + FilePath);
            bool bresult = false;
            try
            {
                if (File.Exists(FilePath))
                {
                    if (new FileInfo(FilePath).Length > 0)
                    {
                        FileType fileType = GetFileType(FilePath);

                        switch (fileType)
                        {
                            case FileType.WORD:
                                {
                                    Log.WriteLine("File Type is Word");
                                    using (Stream streamDocument = new FileStream(FilePath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                                    {
                                        var document = WordprocessingDocument.Open(streamDocument, false);
                                        var customProps = document.CustomFilePropertiesPart;
                                        if (customProps != null)
                                        {
                                            Log.WriteLine("Get Custom Props");
                                            DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                            if (GetTagKeyCount() == 0)
                                            {
                                                Log.WriteLine("Tag key count is 0 , get all tag");
                                                GetPropertiesToTagCollection(props);
                                            }
                                            else
                                            {
                                                Log.WriteLine("Tag key count is not 0 , only get one Tag");
                                                GetSinglePropertiesToTagCollection(props, GetTagKey(0));
                                            }
                                        }
                                        document.Close();
                                    }
                                    bresult = true;
                                }
                                break;
                            case FileType.EXCEL:
                                {
                                    using (Stream streamDocument = new FileStream(FilePath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                                    {
                                        Log.WriteLine("File Type is Excel");
                                        var document = SpreadsheetDocument.Open(streamDocument, false);
                                        var customProps = document.CustomFilePropertiesPart;
                                        if (customProps != null)
                                        {
                                            DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                            if (GetTagKeyCount() == 0)
                                            {
                                                Log.WriteLine("Tag key count is 0 , get all tag");
                                                GetPropertiesToTagCollection(props);
                                            }
                                            else
                                            {
                                                Log.WriteLine("Tag key count is not 0 , only get one Tag");
                                                GetSinglePropertiesToTagCollection(props, GetTagKey(0));
                                            }
                                        }
                                        document.Close();
                                    }
                                    bresult = true;
                                }
                                break;
                            case FileType.POWERPOINT:
                                {
                                    using (Stream streamDocument = new FileStream(FilePath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                                    {
                                        var document = PresentationDocument.Open(streamDocument, false);
                                        var customProps = document.CustomFilePropertiesPart;
                                        if (customProps != null)
                                        {
                                            DocumentFormat.OpenXml.CustomProperties.Properties props = customProps.Properties;
                                            if (GetTagKeyCount() == 0)
                                            {
                                                Log.WriteLine("Tag key count is 0 , get all tag");
                                                GetPropertiesToTagCollection(props);
                                            }
                                            else
                                            {
                                                Log.WriteLine("Tag key count is not 0 , only get one Tag");
                                                GetSinglePropertiesToTagCollection(props, GetTagKey(0));
                                            }
                                        }
                                        document.Close();
                                    }
                                    bresult = true;
                                }
                                break;
                            case FileType.OTHER:
                                {
                                    Log.WriteLine("Type is Other");
                                    bresult = false;
                                }
                                break;
                        }
                    }
                    else
                    {
                        Log.WriteLine("FILE IS SIZE IS ZERO , BUT RETURN TRUE");
                        bresult = true;
                    }
                }
                else
                {
                    Log.WriteLine("FILE IS NOT EXITES:" + FilePath);
                }
            }
            catch (Exception ex)
            {
                Log.WriteLine(ex);
                bresult = false;
            }
            finally
            {
                Log.WriteLine("ExecuteReadCustomTag End result is :" + bresult);
            }
            return bresult;
        }
        public bool ExecuteReadSumrryTag()
        {
            Log.WriteLine("ExecuteReadSumrryTag Start:" + FilePath);
            bool bresult = false;
            try
            {
                if (File.Exists(FilePath))
                {
                    if (new FileInfo(FilePath).Length > 0)
                    {
                        FileType fileType = GetFileType(FilePath);

                        switch (fileType)
                        {
                            case FileType.WORD:
                                {
                                    using (Stream streamDocument = new FileStream(FilePath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                                    {
                                        Log.WriteLine("File Type is Word");
                                        var document = WordprocessingDocument.Open(streamDocument, false);
                                        var packageProps = document.PackageProperties;
                                        if (packageProps != null)
                                        {
                                            Log.WriteLine("Get Custom Props");
                                            GetPropertiesToTagCollection(packageProps);
                                            GetPropertiesToTagCollection(document.ExtendedFilePropertiesPart.Properties);
                                        }
                                        document.Close();
                                    }
                                    bresult = true;
                                }
                                break;
                            case FileType.EXCEL:
                                {
                                    using (Stream streamDocument = new FileStream(FilePath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                                    {
                                        Log.WriteLine("File Type is Excel");
                                        var document = SpreadsheetDocument.Open(streamDocument, false);
                                        var packageProps = document.PackageProperties;
                                        if (packageProps != null)
                                        {
                                            GetPropertiesToTagCollection(packageProps);
                                            GetPropertiesToTagCollection(document.ExtendedFilePropertiesPart.Properties);
                                        }
                                        document.Close();
                                    }
                                    bresult = true;
                                }
                                break;
                            case FileType.POWERPOINT:
                                {
                                    using (Stream streamDocument = new FileStream(FilePath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                                    {
                                        var document = PresentationDocument.Open(streamDocument, false);
                                        var packageProps = document.PackageProperties;
                                        if (packageProps != null)
                                        {
                                            GetPropertiesToTagCollection(packageProps);
                                            GetPropertiesToTagCollection(document.ExtendedFilePropertiesPart.Properties);
                                        }
                                        document.Close();
                                    }
                                    bresult = true;
                                }
                                break;
                            case FileType.OTHER:
                                {
                                    Log.WriteLine("Type is Other");
                                    bresult = false;
                                }
                                break;
                        }
                    }
                    else
                    {
                        Log.WriteLine("FILE IS SIZE IS ZERO , BUT RETURN TRUE");
                        bresult = true;
                    }
                }
                else
                {
                    Log.WriteLine("FILE IS NOT EXITES:" + FilePath);
                }
            }
            catch (Exception ex)
            {
                Log.WriteLine(ex);
                bresult = false;
            }
            finally
            {
                Log.WriteLine("ExecuteReadSumrryTag End result is :" + bresult);
            }
            return bresult;
        }

        private enum FileType : int
        {
            WORD,
            EXCEL,
            POWERPOINT,
            OTHER
        }
        enum PropertyTypes : int
        {
            YesNo,
            Text,
            DateTime,
            NumberInteger,
            NumberDouble
        }

        private static string GetFileSuffix(string strFileName)
        {
            string strSuffix = "";
            int nPos = strFileName.LastIndexOf('.');
            if (nPos >= 0)
            {
                strSuffix = strFileName.Substring(nPos + 1);
            }
            return strSuffix;
        }

        private static FileType GetFileType(string strFileName)
        {
            List<string> wordSuffixs = new List<string>(new string[] { "docx", "docm", "dotx", "dotm" });
            List<string> excelSuffixs = new List<string>(new string[] { "xlsx", "xlsm", "xltm", "xltx" });
            List<string> pptSuffixs = new List<string>(new string[] { "pptx", "potx", "potm", "pptm" });

            //get file suffix
            string strSuffix = GetFileSuffix(strFileName);

            if (wordSuffixs.Contains(strSuffix))
            {
                return FileType.WORD;
            }
            else if (excelSuffixs.Contains(strSuffix))
            {
                return FileType.EXCEL;
            }
            else if (pptSuffixs.Contains(strSuffix))
            {
                return FileType.POWERPOINT;
            }
            else
            {
                return FileType.OTHER;
            }
        }

        private void AddProperties(DocumentFormat.OpenXml.CustomProperties.Properties props, Dictionary<string, List<string>> lstProps)
        {
            Log.WriteLine("AddProperties Start");
            foreach (KeyValuePair<string, List<string>> PropItem in lstProps)
            {

                CustomDocumentProperty newProp = CreateCustomDocProperty(String.Join("|", PropItem.Value.ToArray()), PropertyTypes.Text);

                // Now that you have handled the parameters, start
                // working on the document.
                newProp.FormatId = "{D5CDD505-2E9C-101B-9397-08002B2CF9AE}";
                newProp.Name = PropItem.Key;
                Log.WriteLine("PropItem Key:" + PropItem.Key + " PropItem Value" + PropItem.Value);
                AppendNewProperty(props, newProp);

            }
            Log.WriteLine("AddProperties End");
        }
        private CustomDocumentProperty CreateCustomDocProperty(object propertyValue, PropertyTypes propertyType)
        {
            var newProp = new CustomDocumentProperty();
            bool propSet = false;

            // Calculate the correct type.
            switch (propertyType)
            {
                case PropertyTypes.DateTime:

                    // Be sure you were passed a real date, 
                    // and if so, format in the correct way. 
                    // The date/time value passed in should 
                    // represent a UTC date/time.
                    if ((propertyValue) is DateTime)
                    {
                        newProp.VTFileTime =
                            new VTFileTime(string.Format("{0:s}Z",
                                Convert.ToDateTime(propertyValue)));
                        propSet = true;
                    }

                    break;

                case PropertyTypes.NumberInteger:
                    if ((propertyValue) is int)
                    {
                        newProp.VTInt32 = new VTInt32(propertyValue.ToString());
                        propSet = true;
                    }

                    break;

                case PropertyTypes.NumberDouble:
                    if (propertyValue is double)
                    {
                        newProp.VTFloat = new VTFloat(propertyValue.ToString());
                        propSet = true;
                    }

                    break;

                case PropertyTypes.Text:
                    newProp.VTLPWSTR = new VTLPWSTR(propertyValue.ToString());
                    propSet = true;

                    break;

                case PropertyTypes.YesNo:
                    if (propertyValue is bool)
                    {
                        // Must be lowercase.
                        newProp.VTBool = new VTBool(
                          Convert.ToBoolean(propertyValue).ToString().ToLower());
                        propSet = true;
                    }
                    break;
            }

            if (!propSet)
            {
                // If the code was not able to convert the 
                // property to a valid value, throw an exception.
                throw new InvalidDataException("propertyValue");
            }

            return newProp;
        }
        private void AppendNewProperty(DocumentFormat.OpenXml.CustomProperties.Properties props, CustomDocumentProperty newProp)
        {
            Log.WriteLine("AppendNewProperty Start");
            if (props != null)
            {
                Log.WriteLine("props !=null");
                // This will trigger an exception if the property's Name 
                // property is null, but if that happens, the property is damaged, 
                // and probably should raise an exception.
                var prop = default(CustomDocumentProperty);
                foreach (CustomDocumentProperty tempProp in props)
                {
                    Log.WriteLine("tempProp.Name.Value:" + tempProp.Name.Value);
                    if (tempProp.Name.Value == newProp.Name)
                    {
                        prop = tempProp;
                        break;
                    }
                }

                // Does the property exist? If so, get the return value, 
                // and then delete the property.
                if (prop != null)
                {
                    Log.WriteLine("prop!=null");
                    prop.Remove();
                    Log.WriteLine("prop.Remove()");
                }

                // Append the new property, and 
                // fix up all the property ID values. 
                // The PropertyId value must start at 2.
                props.AppendChild(newProp);
                Log.WriteLine("props.AppendChild(newProp)");
                int pid = 2;
                foreach (CustomDocumentProperty item in props)
                {
                    item.PropertyId = pid++;
                }
                props.Save();
                Log.WriteLine("props.Save()");
            }
            Log.WriteLine("AppendNewProperty End");
        }

        private List<CustomDocumentProperty> GetPropsChildrenFromTagCollection(Dictionary<string, List<string>> dicTags)
        {
            Log.WriteLine("GetPropsChildrenFromTagCollection Start");
            List<CustomDocumentProperty> lisCustomProps = new List<CustomDocumentProperty>();
            foreach (KeyValuePair<string, List<string>> PropItem in dicTags)
            {
                CustomDocumentProperty newProp = CreateCustomDocProperty(String.Join("|", PropItem.Value.ToArray()), PropertyTypes.Text);
                // Now that you have handled the parameters, start
                // working on the document.
                newProp.FormatId = "{D5CDD505-2E9C-101B-9397-08002B2CF9AE}";
                newProp.Name = PropItem.Key;
                Log.WriteLine("PropItem Key:" + PropItem.Key + " PropItem Value" + PropItem.Value);
                lisCustomProps.Add(newProp);
            }
            Log.WriteLine("GetPropsChildrenFromTagCollection End");
            return lisCustomProps;
        }
        private void RemovePropertiesByTagCollection(DocumentFormat.OpenXml.CustomProperties.Properties props, Dictionary<string, List<string>> tagCollection)
        {
            List<CustomDocumentProperty> lisNeedDeleteProps = new List<CustomDocumentProperty>();
            List<CustomDocumentProperty> lisNeedAddProps = new List<CustomDocumentProperty>();
            foreach (CustomDocumentProperty customProperty in props.ChildElements)
            {
                foreach (string strTagKey in tagCollection.Keys)
                {
                    if (customProperty.Name != null)
                    {
                        Log.WriteLine("customProperty Name is :" + customProperty.Name.ToString());
                        Log.WriteLine("Need Remove Tag Key is :" + strTagKey);
                        if (customProperty.Name.ToString().Equals(strTagKey, StringComparison.OrdinalIgnoreCase))
                        {
                            Log.WriteLine("Key Matched");
                            Log.WriteLine("Value Count:" + tagCollection[strTagKey].Count);
                            if (tagCollection[strTagKey].Count == 0)
                            {
                                Log.WriteLine("Remove this property ignone value");
                                lisNeedDeleteProps.Add(customProperty);
                            }
                            else
                            {
                                Log.WriteLine("Check this property value");
                                if (customProperty.InnerText != null)
                                {
                                    Log.WriteLine("Property's value is :" + customProperty.InnerText);
                                    List<string> lisCustomPropertyValue = customProperty.InnerText.Split('|').ToList<string>();
                                    foreach (string strTagValue in tagCollection[strTagKey])
                                    {
                                        Log.WriteLine("Remove Value:" + strTagValue);
                                        lisCustomPropertyValue.Remove(strTagValue);
                                    }
                                    CustomDocumentProperty newProp = CreateCustomDocProperty(String.Join("|", lisCustomPropertyValue.ToArray()), PropertyTypes.Text);
                                    newProp.FormatId = "{D5CDD505-2E9C-101B-9397-08002B2CF9AE}";
                                    newProp.Name = strTagKey;
                                    lisNeedAddProps.Add(newProp);
                                    lisNeedDeleteProps.Add(customProperty);
                                    Log.WriteLine("Result value is :" + String.Join("|", lisCustomPropertyValue.ToArray()));
                                }
                            }
                        }
                    }
                }
            }
            foreach (CustomDocumentProperty Property in lisNeedDeleteProps)
            {
                props.RemoveChild(Property);
            }
            foreach (CustomDocumentProperty Property in lisNeedAddProps)
            {
                AppendNewProperty(props, Property);
            }
        }

        private void UpdatePropertiesByTagCollection(DocumentFormat.OpenXml.CustomProperties.Properties props, Dictionary<string, List<string>> tagCollection)
        {
            List<CustomDocumentProperty> lisNeedDeleteProps = new List<CustomDocumentProperty>();
            List<CustomDocumentProperty> lisNeedAddProps = new List<CustomDocumentProperty>();
            foreach (CustomDocumentProperty customProperty in props.ChildElements)
            {
                foreach (string strTagKey in tagCollection.Keys)
                {
                    if (customProperty.Name != null)
                    {
                        Log.WriteLine("customProperty Name is :" + customProperty.Name.ToString());
                        Log.WriteLine("Need Remove Tag Key is :" + strTagKey);
                        if (customProperty.Name.ToString().Equals(strTagKey, StringComparison.OrdinalIgnoreCase))
                        {
                            Log.WriteLine("Key Matched");
                            Log.WriteLine("Value Count:" + tagCollection[strTagKey].Count);
                            if (tagCollection[strTagKey].Count == 0)
                            {
                                Log.WriteLine("Update must have value!");
                            }
                            else
                            {
                                Log.WriteLine("Check this property value");
                                if (customProperty.InnerText != null)
                                {
                                    Log.WriteLine("Property's value is :" + customProperty.InnerText);

                                    CustomDocumentProperty newProp = CreateCustomDocProperty(String.Join("|", tagCollection[strTagKey].ToArray()), PropertyTypes.Text);
                                    newProp.FormatId = "{D5CDD505-2E9C-101B-9397-08002B2CF9AE}";
                                    newProp.Name = strTagKey;
                                    lisNeedAddProps.Add(newProp);
                                    lisNeedDeleteProps.Add(customProperty);
                                    Log.WriteLine("Result value is :" + String.Join("|", tagCollection[strTagKey].ToArray()));
                                }
                            }
                        }
                    }
                }
            }
            foreach (CustomDocumentProperty Property in lisNeedDeleteProps)
            {
                props.RemoveChild(Property);
            }
            foreach (CustomDocumentProperty Property in lisNeedAddProps)
            {
                AppendNewProperty(props, Property);
            }
        }

        private void AddPropertiesFormTagCollection(DocumentFormat.OpenXml.CustomProperties.Properties props, Dictionary<string, List<string>> tagCollection, bool bOverWrite)
        {
            List<CustomDocumentProperty> lisNeedDeleteProps = new List<CustomDocumentProperty>();
            foreach (CustomDocumentProperty customProperty in props.ChildElements)
            {
                foreach (string strTagKey in tagCollection.Keys)
                {
                    if (customProperty.Name != null)
                    {
                        if (customProperty.Name.ToString().Equals(strTagKey, StringComparison.OrdinalIgnoreCase))
                        {
                            lisNeedDeleteProps.Add(customProperty);
                            if (!bOverWrite)
                            {
                                if (customProperty.InnerText != null)
                                {
                                    List<string> lisOldTagVlue = customProperty.InnerText.Split('|').ToList<string>();
                                    foreach (string strOldTagValue in lisOldTagVlue)
                                    {
                                        if (!tagCollection[strTagKey].Contains(strOldTagValue))
                                        {
                                            tagCollection[strTagKey].Add(strOldTagValue);
                                        }
                                    }
                                }
                            }


                        }
                    }
                }
            }
            foreach (CustomDocumentProperty Property in lisNeedDeleteProps)
            {
                props.RemoveChild(Property);
            }
            foreach (CustomDocumentProperty Property in GetPropsChildrenFromTagCollection(tagCollection))
            {
                AppendNewProperty(props, Property);
            }
        }


        private void GetPropertiesToTagCollection(DocumentFormat.OpenXml.CustomProperties.Properties props)
        {
            foreach (CustomDocumentProperty customProperty in props.ChildElements)
            {
                if (customProperty.Name != null)
                {
                    if (customProperty.InnerText != null)
                    {
                        List<string> lisCustomPropertyValue = customProperty.InnerText.Split('|').ToList<string>();
                        foreach (string strValue in lisCustomPropertyValue)
                        {
                            SetTag(customProperty.Name.ToString(), strValue);
                        }
                    }
                }
            }
        }
        private void GetSinglePropertiesToTagCollection(DocumentFormat.OpenXml.CustomProperties.Properties props, string strPropertyName)
        {
            Log.WriteLine("GetSinglePropertiesToTagCollection Start");
            foreach (CustomDocumentProperty customProperty in props.ChildElements)
            {
                if (customProperty.Name != null)
                {
                    if (strPropertyName.Equals(customProperty.Name, StringComparison.OrdinalIgnoreCase))
                    {
                        if (customProperty.InnerText != null)
                        {
                            List<string> lisCustomPropertyValue = customProperty.InnerText.Split('|').ToList<string>();
                            SetTag(strPropertyName, lisCustomPropertyValue[0]);
                            Log.WriteLine("Find SingleTag key:" + strPropertyName + " value:" + lisCustomPropertyValue[0]);
                            break;
                        }
                    }
                }
            }
            Log.WriteLine("GetSinglePropertiesToTagCollection End");
        }
        private void GetPropertiesToTagCollection(System.IO.Packaging.PackageProperties props)
        {
            SetTag(STR_SUMMARY_TAG_TITLE_NAME, props.Title);
            SetTag(STR_SUMMARY_TAG_SUBJECT_NAME, props.Subject);
            SetTag(STR_SUMMARY_TAG_AUTHOR_NAME, props.Creator);
            SetTag(STR_SUMMARY_TAG_CATEGORY_NAME, props.Category);
            SetTag(STR_SUMMARY_TAG_KEYWORDS_NAME, props.Keywords);
            SetTag(STR_SUMMARY_TAG_COMMENTS_NAME, props.Description);
        }
        private void GetPropertiesToTagCollection(DocumentFormat.OpenXml.ExtendedProperties.Properties pops)
        {

            foreach (var property in pops)
            {

                switch (property.LocalName)
                {
                    case STR_SUMMARY_TAG_MANAGE_NAME:
                        {
                            Log.WriteLine(STR_SUMMARY_TAG_MANAGE_NAME + ":" + property.InnerText);
                            SetTag(STR_SUMMARY_TAG_MANAGE_NAME, property.InnerText);
                        }
                        break;
                    case STR_SUMMARY_TAG_COMPANY_NAME:
                        {
                            Log.WriteLine(STR_SUMMARY_TAG_COMPANY_NAME + ":" + property.InnerText);
                            SetTag(STR_SUMMARY_TAG_COMPANY_NAME, property.InnerText);
                        }
                        break;
                    case STR_SUMMARY_TAG_HYPERLINKBASE_NAME:
                        {
                            Log.WriteLine(STR_SUMMARY_TAG_HYPERLINKBASE_NAME + ":" + property.InnerText);
                            SetTag(STR_SUMMARY_TAG_HYPERLINKBASE_NAME, property.InnerText);
                        }
                        break;
                    case STR_SUMMARY_TAG_TEMPLATE_NAME:
                        {
                            Log.WriteLine(STR_SUMMARY_TAG_HYPERLINKBASE_NAME + ":" + property.InnerText);
                            SetTag(STR_SUMMARY_TAG_HYPERLINKBASE_NAME, property.InnerText);
                        }
                        break;
                    default:
                        {
                            Log.WriteLine("Default:" + ":" + property.LocalName);
                        }
                        break;
                }
            }
        }
    }
}
