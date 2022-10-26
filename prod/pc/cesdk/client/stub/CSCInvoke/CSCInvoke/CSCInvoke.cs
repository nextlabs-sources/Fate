//
// All sources, binaries and HTML pages (C) copyright 2007 by NextLabs Inc. 
// San Mateo CA, Ownership remains with NextLabs Inc, 
// All rights reserved worldwide. 
//
//
// NextLabs Compliant Enterprise SDK in .NET
// 
// <-------------------------------------------------------------------------->

using System;
using System.Threading;
using System.Runtime.InteropServices;
using System.Text;
using System.Diagnostics;
namespace CETYPE
{
  public enum CEResult_t
  {
    CE_RESULT_SUCCESS = 0,
    CE_RESULT_GENERAL_FAILED = -1,
    CE_RESULT_CONN_FAILED = -2,
    CE_RESULT_INVALID_PARAMS = -3,
    CE_RESULT_VERSION_MISMATCH = -4,
    CE_RESULT_FILE_NOT_PROTECTED = -5,
    CE_RESULT_INVALID_PROCESS = -6,
    CE_RESULT_INVALID_COMBINATION = -7,
    CE_RESULT_PERMISSION_DENIED = -8,
    CE_RESULT_FILE_NOT_FOUND = -9,
    CE_RESULT_FUNCTION_NOT_AVAILBLE = -10,
    CE_RESULT_TIMEDOUT = -11,
    CE_RESULT_SHUTDOWN_FAILED = -12, 
    CE_RESULT_INVALID_ACTION_ENUM = -13, 
    CE_RESULT_EMPTY_SOURCE = -14, 
    CE_RESULT_MISSING_MODIFIED_DATE = -15, 
    CE_RESULT_NULL_CEHANDLE = -16,
    CE_RESULT_INVALID_EVAL_ACTION = -17, 
    CE_RESULT_EMPTY_SOURCE_ATTR = -18, 
    CE_RESULT_EMPTY_ATTR_KEY = -19, 
    CE_RESULT_EMPTY_ATTR_VALUE = -20, 
    CE_RESULT_EMPTY_PORTAL_USER = -21, 
    CE_RESULT_EMPTY_PORTAL_USERID = -22, 
    CE_RESULT_MISSING_TARGET = -23, 
    CE_RESULT_PROTECTION_OBJECT_NOT_FOUND = -24,
    CE_RESULT_NOT_SUPPORTED = -25,
    CE_RESULT_SERVICE_NOT_READY = -26,
    CE_RESULT_SERVICE_NOT_FOUND = -27,
    CE_RESULT_INSUFFICIENT_BUFFER = -28,
    CE_RESULT_ALREADY_EXISTS = -29,
    CE_RESULT_APPLICATION_AUTH_FAILED = -30,
    CE_RESULT_RECURSIVE_INVOCATION = -31,
    CE_RESULT_TOO_MANY_REQUESTS = -32
  };

  public enum CEResponse_t
  {
    CEDeny = 0,
    CEAllow = 1
  };

  public enum CENoiseLevel_t 
  {
    CE_NOISE_LEVEL_MIN = 0,
    CE_NOISE_LEVEL_SYSTEM = 1,
    CE_NOISE_LEVEL_APPLICATION = 2,
    CE_NOISE_LEVEL_USER_ACTION = 3,
    CE_NOISE_LEVEL_MAX = 4
  };

  public enum CEAction_t
  {
    CE_ACTION_READ = 1,
    CE_ACTION_DELETE = 2,
    CE_ACTION_MOVE = 3,
    CE_ACTION_COPY = 4,
    CE_ACTION_WRITE = 5,
    CE_ACTION_RENAME = 6,
    CE_ACTION_CHANGE_ATTR_FILE = 7,
    CE_ACTION_CHANGE_SEC_FILE = 8,
    CE_ACTION_PRINT_FILE = 9,
    CE_ACTION_PASTE_FILE = 10,
    CE_ACTION_EMAIL_FILE = 11,
    CE_ACTION_IM_FILE = 12,
    CE_ACTION_EXPORT = 13,
    CE_ACTION_IMPORT = 14,
    CE_ACTION_CHECKIN = 15,
    CE_ACTION_CHECKOUT = 16,
    CE_ACTION_ATTACH = 17,
    CE_ACTION_RUN = 18,
    CE_ACTION_REPLY = 19, 
    CE_ACTION_FORWARD = 20, 
    CE_ACTION_NEW_EMAIL = 21, 
    CE_ACTION_AVD = 22, 
    CE_ACTION_MEETING = 23,
    CE_ACTION_PROCESS_TERMINATE = 24,
    CE_ACTION_WM_SHARE = 25, 
    CE_ACTION_WM_RECORD = 26, 
    CE_ACTION_WM_QUESTION = 27, 
    CE_ACTION_WM_VOICE = 28, 
    CE_ACTION_WM_VIDEO = 29, 
    CE_ACTION_WM_JOIN = 30  
  }
    
  public struct CEApplication
  {
    public string appName;
    public string appPath;
    public string appURL;
    
    public CEApplication(string n, string p, string u)
    {
      appName = n;
      appPath = p;
      appURL = u;
    }
  }

  public struct CEUser
  {
    public string userName;
    public string userID;
    
    public CEUser(string n, string i)
    {
      userName = n;
      userID = i;
    }
  }

  public struct CENamedAttributes
  {
    public string name;
    public string[] attrs;

    public CENamedAttributes(string name, string[] attrs)
    {
      this.name = name;
      this.attrs = attrs;
    }
  }

  public struct CERequest
  {
    public string operation;
    public CEResource source;
    public string[] sourceAttributes;
    public CEResource target;
    public string[] targetAttributes;
    public CEUser user;
    public string[] userAttributes;
    public CEApplication application;
    public string[] applicationAttributes;
    public string[] recipients;
    public CENamedAttributes[] additionalAttributes;
    public bool performObligations;
    public CENoiseLevel_t noiseLevel;

    public CERequest(string operation,
                     CEResource source,
                     string[] sourceAttributes,
                     CEResource target,
                     string[] targetAttributes,
                     CEUser user,
                     string[] userAttributes,
                     CEApplication application,
                     string[] applicationAttributes,
                     string[] recipients,
                     CENamedAttributes[] additionalAttributes,
                     bool performObligations,
                     CENoiseLevel_t noiseLevel)
    {
      this.operation = operation;
      this.source = source;
      this.sourceAttributes = sourceAttributes;
      this.target = target;
      this.targetAttributes = targetAttributes;
      this.user = user;
      this.userAttributes = userAttributes;
      this.application = application;
      this.applicationAttributes = applicationAttributes;
      this.recipients = recipients;
      this.additionalAttributes = additionalAttributes;
      this.performObligations = performObligations;
      this.noiseLevel = noiseLevel;
    }
  }

  public class CEAttrKey
  {
    public const string CE_ATTR_MODIFIED_DATE = "modified_date";
    public const string CE_ATTR_SP_NAME = "name";
    public const string CE_ATTR_SP_TITLE = "title";
    public const string CE_ATTR_SP_DESC = "desc";
    public const string CE_ATTR_SP_RESOURCE_TYPE = "type";
    public const string CE_ATTR_SP_RESOURCE_SUBTYPE = "sub_type";
    public const string CE_ATTR_SP_CREATED_BY = "created_by";
    public const string CE_ATTR_SP_MODIFIED_BY = "modified_by";
    public const string CE_ATTR_SP_DATE_CREATED = "created";
    public const string CE_ATTR_SP_DATE_MODIFIED = "modified";
    public const string CE_ATTR_SP_FILE_SIZE = "file_size";

    public const string CE_ATTR_OBLIGATION_COUNT  = "CE_ATTR_OBLIGATION_COUNT";
    public const string CE_ATTR_OBLIGATION_NAME   = "CE_ATTR_OBLIGATION_NAME";
    public const string CE_ATTR_OBLIGATION_POLICY = "CE_ATTR_OBLIGATION_POLICY";
    public const string CE_ATTR_OBLIGATION_VALUE  = "CE_ATTR_OBLIGATION_VALUE";
  }

  public class CEAttrVal
  {
    public const string CE_ATTR_SP_TYPE_VAL_SITE        = "site";
    public const string CE_ATTR_SP_TYPE_VAL_PORTLET     = "portlet";
    public const string CE_ATTR_SP_TYPE_VAL_ITEM        = "item";
    public const string CE_ATTR_SP_SUBTYPE_VAL_SITE     = "site";
    public const string CE_ATTR_SP_SUBTYPE_VAL_LIST     = "list";
    public const string CE_ATTR_SP_SUBTYPE_VAL_LIBRARY  = "library";
    public const string CE_ATTR_SP_SUBTYPE_VAL_LIST_ITEM = "list item";
    public const string CE_ATTR_SP_SUBTYPE_VAL_LIBRARY_ITEM="library item";

    public const string CE_OBLIGATION_NOTIFY            = "CE::NOTIFY";
  }

  public class CEResource
  {
    public string resourceName;
    public string resourceType;
    
    public CEResource(string n, string t)
    {
      resourceName = n;
      resourceType = t;
    }
  }

  public class CEEnforcement
  {
    public CETYPE.CEResponse_t result;
    public string[] obligations;

    public CEEnforcement(int result, string[] obligations)
    {
      this.result = (CETYPE.CEResponse_t)result;
      this.obligations = obligations;
    }
  }
}

namespace NextLabs.CSCInvoke
{
  [StructLayout(LayoutKind.Sequential)]
  public class CEApplication : IDisposable
  {
    private IntPtr appName;
    private IntPtr appPath;
    private IntPtr appURL;

    public static int size = Marshal.SizeOf(typeof(CEApplication));

    public CEApplication()
    {
      appName = IntPtr.Zero;
      appPath = IntPtr.Zero;
      appURL = IntPtr.Zero;
    }

    public CEApplication(CETYPE.CEApplication app)
    {
      if (appName != null)
      {
        this.appName = Marshal.AllocCoTaskMem(CEString.size);
        Marshal.StructureToPtr(new CEString(app.appName), this.appName, false);
      }
      else
      {
        this.appName = IntPtr.Zero;
      }

      if (appPath != null)
      {
        this.appPath = Marshal.AllocCoTaskMem(CEString.size);
        Marshal.StructureToPtr(new CEString(app.appPath), this.appPath, false);
      }
      else
      {
        this.appPath = IntPtr.Zero;
      }

      if (appURL != null)
      {
        this.appURL = Marshal.AllocCoTaskMem(CEString.size);
        Marshal.StructureToPtr(new CEString(app.appURL), this.appURL, false);
      }
      else
      {
        this.appURL = IntPtr.Zero;
      }
    }

    public void Dispose()
    {
      if (this.appName != IntPtr.Zero)
      {
        Marshal.DestroyStructure(this.appName, typeof(CEString));
        Marshal.FreeCoTaskMem(this.appName);
      }

      if (this.appPath != IntPtr.Zero)
      {
        Marshal.DestroyStructure(this.appPath, typeof(CEString));
        Marshal.FreeCoTaskMem(this.appPath);
      }

      if (this.appURL != IntPtr.Zero)
      {
        Marshal.DestroyStructure(this.appURL, typeof(CEString));
        Marshal.FreeCoTaskMem(this.appURL);
      }
    }
  }

  [StructLayout(LayoutKind.Sequential)]
  public class CEAttribute : IDisposable
  {
    private IntPtr key;    // CEString
    private IntPtr value;  // CEString

    public static int size = Marshal.SizeOf(typeof(CEAttribute));

    public CEAttribute()
    {
      key = IntPtr.Zero;
      value = IntPtr.Zero;
            
    }

    public CEAttribute(string key, string value)
    {
      this.key = Marshal.AllocCoTaskMem(CEString.size);
      Marshal.StructureToPtr(new CEString(key), this.key, false);

      this.value = Marshal.AllocCoTaskMem(CEString.size);
      Marshal.StructureToPtr(new CEString(value), this.value, false);
    }

    public void Dispose()
    {
      Marshal.DestroyStructure(key, typeof(CEString));
      Marshal.FreeCoTaskMem(key);
      Marshal.DestroyStructure(value, typeof(CEString));
      Marshal.FreeCoTaskMem(value);
    }
  }

  [StructLayout(LayoutKind.Sequential)]
  public class CEAttributes : IDisposable
  {
    private IntPtr attributes;
    private int count;

    public static int size = Marshal.SizeOf(typeof(CEAttributes));

    public CEAttributes()
    {
      attributes = IntPtr.Zero;
    }

    public CEAttributes(string[] attrs)
    {
      if (attrs == null)
      {
        attributes = IntPtr.Zero;
        count = 0;
      }
      else
      {
        count = attrs.Length/2;
      
        attributes = Marshal.AllocCoTaskMem(CEAttribute.size * count);
        for (int i = 0; i < attrs.Length; i+=2)
        {
          CEAttribute attr = new CEAttribute(attrs[i], attrs[i+1]);

          IntPtr writeTo = new IntPtr(attributes.ToInt32()+(i/2)*CEAttribute.size);

          Marshal.StructureToPtr(attr, writeTo, false);
        }
      }
    }

    public void Dispose()
    {
      for (int i = 0; i < count; i++)
      {
        IntPtr readFrom = new IntPtr(attributes.ToInt32() + i * CEAttribute.size);
        CEAttribute attr = (CEAttribute)Marshal.PtrToStructure(readFrom, typeof(CEAttribute));

        attr.Dispose();
      }

      if (attributes != IntPtr.Zero)
      {
        Marshal.FreeCoTaskMem(attributes);
      }
    }
  }

  [StructLayout(LayoutKind.Sequential)]
  public struct CENamedAttributes : IDisposable
  {
    private IntPtr name;   // CEString
    private CEAttributes attrs;

    public static int size = Marshal.SizeOf(typeof(CENamedAttributes));

    public CENamedAttributes(CETYPE.CENamedAttributes namedAttributes) : this(namedAttributes.name, namedAttributes.attrs)
    {
    }

    public CENamedAttributes(string name, string[] attributes)
    {
      CEString attributesName = new CEString(name);

      this.name = Marshal.AllocCoTaskMem(Marshal.SizeOf(typeof(CEString)));
      Marshal.StructureToPtr(attributesName, this.name, false);
      
      this.attrs = new CEAttributes(attributes);
    }

    public void Dispose()
    {
      Marshal.DestroyStructure(this.name, typeof(CEString));
      Marshal.FreeCoTaskMem(this.name);
      attrs.Dispose();
    }
  }

  [StructLayout(LayoutKind.Sequential)]
  public struct CERequest : IDisposable
  {
    private IntPtr operation;        // CEString
    private IntPtr source;           // CEResource
    private IntPtr sourceAttributes; // CEAttributes
    private IntPtr target;           // CEResource
    private IntPtr targetAttributes; // CEAttributes
    private IntPtr user;             // CEUser
    private IntPtr userAttributes;   // CEAttributes
    private IntPtr application;      // CEApplication
    private IntPtr applicationAttributes; // CEAttributes
    private IntPtr recipients;       // CEString array
    private int numRecipients;
    private IntPtr additionalAttributes; // CENamedAttributes array
    private int numAdditionalAttributes;
    private bool performObligations;
    private CETYPE.CENoiseLevel_t noiseLevel;

    public static int size = Marshal.SizeOf(typeof(CERequest));

    public CERequest(CETYPE.CERequest request)
    {
      CEString ceOperation = new CEString(request.operation);
      this.operation = Marshal.AllocCoTaskMem(CEString.size);
      Marshal.StructureToPtr(ceOperation, this.operation, false);

      this.source = Marshal.AllocCoTaskMem(CEString.size);
      Marshal.StructureToPtr(new CEResource(request.source), this.source, false);

      if (request.sourceAttributes != null)
      {
        this.sourceAttributes = Marshal.AllocCoTaskMem(CEAttributes.size);
        Marshal.StructureToPtr(new CEAttributes(request.sourceAttributes), this.sourceAttributes, false);
      }
      else
      {
        this.sourceAttributes = IntPtr.Zero;
      }

      if (request.target != null)
      {
        this.target = Marshal.AllocCoTaskMem(CEResource.size);
        Marshal.StructureToPtr(new CEResource(request.target), this.target, false);
      }
      else
      {
        this.target = IntPtr.Zero;
      }

      if (request.targetAttributes != null)
      {
        this.targetAttributes = Marshal.AllocCoTaskMem(CEAttributes.size);
        Marshal.StructureToPtr(new CEAttributes(request.targetAttributes), this.targetAttributes, false);
      }
      else
      {
        this.targetAttributes = IntPtr.Zero;
      }

      this.user = Marshal.AllocCoTaskMem(CEUser.size);
      Marshal.StructureToPtr(new CEUser(request.user), this.user, false);

      if (request.userAttributes != null)
      {
        this.userAttributes = Marshal.AllocCoTaskMem(CEAttributes.size);
        Marshal.StructureToPtr(new CEAttributes(request.userAttributes), this.userAttributes, false);
      }
      else
      {
        this.userAttributes = IntPtr.Zero;
      }

      this.application = Marshal.AllocCoTaskMem(CEApplication.size);
      Marshal.StructureToPtr(new CEApplication(request.application), this.application, false);

      if (request.applicationAttributes != null) 
      {
        this.applicationAttributes = Marshal.AllocCoTaskMem(CEAttributes.size);
        Marshal.StructureToPtr(new CEAttributes(request.applicationAttributes), this.applicationAttributes, false);
      }
      else
      {
        this.applicationAttributes = IntPtr.Zero;
      }

      if (request.recipients != null && request.recipients.Length != 0)
      {
        this.numRecipients = request.recipients.Length;
        this.recipients = Marshal.AllocCoTaskMem(Marshal.SizeOf(typeof(IntPtr)) * request.recipients.Length);

        for (int i = 0; i < request.recipients.Length; i++)
        {
          CEString recipient = new CEString(request.recipients[i]);
          IntPtr ptrRecipient = Marshal.AllocCoTaskMem(CEString.size);
          Marshal.StructureToPtr(recipient, ptrRecipient, false);

          IntPtr writeTo = new IntPtr(this.recipients.ToInt32() + i * Marshal.SizeOf(typeof(IntPtr)));
          Marshal.StructureToPtr(ptrRecipient, writeTo, false);
        }
      }
      else
      {
        this.recipients = IntPtr.Zero;
        this.numRecipients = 0;
      }

      if (request.additionalAttributes != null && request.additionalAttributes.Length != 0)
      {
        this.numAdditionalAttributes = request.additionalAttributes.Length;
        this.additionalAttributes = Marshal.AllocCoTaskMem(Marshal.SizeOf(typeof(CENamedAttributes)) * request.additionalAttributes.Length);

        for (int i = 0; i < request.additionalAttributes.Length; i++)
        {
          IntPtr writeTo = new IntPtr(this.additionalAttributes.ToInt32() + i * CENamedAttributes.size);
          Marshal.StructureToPtr(new CENamedAttributes(request.additionalAttributes[i]), writeTo, false);
        }
      }
      else
      {
        this.additionalAttributes = IntPtr.Zero;
        this.numAdditionalAttributes = 0;
      }

      this.performObligations = request.performObligations;
      this.noiseLevel = request.noiseLevel;
    }

    public void Dispose()
    {
      if (operation != IntPtr.Zero)
      {
        Marshal.DestroyStructure(operation, typeof(CEString));
        Marshal.FreeCoTaskMem(operation);
      }

      if (source != IntPtr.Zero)
      {
        CEResource sr = (CEResource)Marshal.PtrToStructure(source, typeof(CEResource));
        sr.Dispose();
        Marshal.DestroyStructure(source, typeof(CEResource));
        Marshal.FreeCoTaskMem(source);
      }

      if (sourceAttributes != IntPtr.Zero)
      {
        CEAttributes sa = (CEAttributes)Marshal.PtrToStructure(sourceAttributes, typeof(CEAttributes));
        sa.Dispose();
        Marshal.DestroyStructure(sourceAttributes, typeof(CEAttributes));
        Marshal.FreeCoTaskMem(sourceAttributes);
      }

      if (target != IntPtr.Zero)
      {
        CEResource tr = (CEResource)Marshal.PtrToStructure(target, typeof(CEResource));
        tr.Dispose();
        Marshal.DestroyStructure(target, typeof(CEResource));
        Marshal.FreeCoTaskMem(target);
      }

      if (targetAttributes != IntPtr.Zero)
      {
        CEAttributes ta = (CEAttributes)Marshal.PtrToStructure(targetAttributes, typeof(CEAttributes));
        ta.Dispose();
        Marshal.DestroyStructure(targetAttributes, typeof(CEAttributes));
        Marshal.FreeCoTaskMem(targetAttributes);
      }

      if (user != IntPtr.Zero)
      {
        CEUser u = (CEUser)Marshal.PtrToStructure(user, typeof(CEUser));
        u.Dispose();
        Marshal.DestroyStructure(user, typeof(CEUser));
        Marshal.FreeCoTaskMem(user);
      }

      if (userAttributes != IntPtr.Zero)
      {
        CEAttributes ua = (CEAttributes)Marshal.PtrToStructure(userAttributes, typeof(CEAttributes));
        ua.Dispose();
        Marshal.DestroyStructure(userAttributes, typeof(CEAttributes));
        Marshal.FreeCoTaskMem(userAttributes);
      }

      if (application != IntPtr.Zero)
      {
        CEApplication app = (CEApplication)Marshal.PtrToStructure(application, typeof(CEApplication));
        app.Dispose();
        Marshal.DestroyStructure(application, typeof(CEApplication));
        Marshal.FreeCoTaskMem(application);
      }

      if (applicationAttributes != IntPtr.Zero)
      {
        CEAttributes aa = (CEAttributes)Marshal.PtrToStructure(applicationAttributes, typeof(CEAttributes));
        aa.Dispose();
        Marshal.DestroyStructure(applicationAttributes, typeof(CEAttributes));
        Marshal.FreeCoTaskMem(applicationAttributes);
      }


      if (recipients != IntPtr.Zero)
      {
        for (int i = 0; i < numRecipients; i++)
        {
          IntPtr recipientPtr = new IntPtr(recipients.ToInt32() + i * Marshal.SizeOf(typeof(IntPtr)));
          IntPtr recipient = (IntPtr)Marshal.PtrToStructure(recipientPtr, typeof(IntPtr));
          
          Marshal.DestroyStructure(recipient, typeof(CEString));
          Marshal.FreeCoTaskMem(recipient);
        }

        Marshal.FreeCoTaskMem(recipients);
      }
        
      if (additionalAttributes != IntPtr.Zero)
      {
        for (int i = 0; i < numAdditionalAttributes; i++)
        {
          IntPtr namedPtr = new IntPtr(additionalAttributes.ToInt32() + i * CENamedAttributes.size);
          CENamedAttributes named = (CENamedAttributes)Marshal.PtrToStructure(namedPtr, typeof(CENamedAttributes));
          named.Dispose();
        }
        Marshal.FreeCoTaskMem(additionalAttributes);
      }
    }
  }

  [StructLayout(LayoutKind.Sequential)]
  public class CEResource : IDisposable
  {
    public IntPtr resourceType;
    public IntPtr resourceName;

    public static int size = Marshal.SizeOf(typeof(CEResource));

    public CEResource()
    {
      resourceName = IntPtr.Zero;
      resourceType = IntPtr.Zero;
    }

    public CEResource(CETYPE.CEResource res)
    {
      this.resourceName = Marshal.AllocCoTaskMem(CEString.size);
      Marshal.StructureToPtr(new CEString(res.resourceName), this.resourceName, false);

      this.resourceType = Marshal.AllocCoTaskMem(CEString.size);
      Marshal.StructureToPtr(new CEString(res.resourceType), this.resourceType, false);
    }

    public void Dispose()
    {
      Marshal.DestroyStructure(resourceName, typeof(CEString));
      Marshal.FreeCoTaskMem(resourceName);
      Marshal.DestroyStructure(resourceType, typeof(CEString));
      Marshal.FreeCoTaskMem(resourceType);
    }
  }

  [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
  public class CEString
  {
    public string buf;
    public int length;

    public static int size = Marshal.SizeOf(typeof(CEString));

    public CEString(string s)
    {
      buf = s;

      if (s == null)
      {
        length = 0;
      }
      else
      {
        length = s.Length;
      }
    }
  }

  [StructLayout(LayoutKind.Sequential)]
  public class CEUser : IDisposable
  {
    private IntPtr userName; // CEString
    private IntPtr userID;   // CEString

    public static int size = Marshal.SizeOf(typeof(CEUser));

    public CEUser()
    {
      userName = IntPtr.Zero;
      userID = IntPtr.Zero;
    }

    public CEUser(CETYPE.CEUser user)
    {
      if (userName != null)
      {
        this.userName = Marshal.AllocCoTaskMem(CEString.size);
        Marshal.StructureToPtr(new CEString(user.userName), this.userName, false);
      }
      else
      {
        this.userName = IntPtr.Zero;
      }

      if (userID != null)
      {
        this.userID = Marshal.AllocCoTaskMem(CEString.size);
        Marshal.StructureToPtr(new CEString(user.userID), this.userID, false);
      }
      else
      {
        this.userID = IntPtr.Zero;
      }
    }

    public void Dispose()
    {
      if (userName != IntPtr.Zero)
      {
        Marshal.DestroyStructure(userName, typeof(CEString));
        Marshal.FreeCoTaskMem(userName);
      }

      if (userID != IntPtr.Zero)
      {
        Marshal.DestroyStructure(userID, typeof(CEString));
        Marshal.FreeCoTaskMem(userID);
      }
    }
  }

  //Declare CE SDK C API for 32 bit cesdk.dll
  public class CESDKAPI_Signature32
  {
    [DllImport("cesdk32.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CECONN_Initialize(
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString binaryPath,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userID,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString pdpHost,
      [Out]out IntPtr connectHandle,
      [In]int timeout_in_millisec);
    
    [DllImport("cesdk32.dll")]
    public static extern CETYPE.CEResult_t CECONN_Close(
      [In]IntPtr handle,
      [In]int timeout_in_millisec);

    [DllImport("cesdk32.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckPortal(
      [In]IntPtr handle,
      [In]CETYPE.CEAction_t operation,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString sourceURL,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] sourceAttributes,
      [In]int numSourceAttributes,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString targetURL,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] targetAttributes,
      [In]int numTargetAttributes,
      [In]uint ipAddress,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userName, 
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userID, 
      [In]bool performObligation,
      [In]CETYPE.CENoiseLevel_t noiseLevel,
      [Out]out IntPtr enforcement_ob,
      [Out]out CETYPE.CEResponse_t enforcement_result,
      [Out]out int numEnforcements,
      [In] int timeout_in_millisec);

    [DllImport("cesdk32.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckFile(
      [In]IntPtr handle,
      [In]CETYPE.CEAction_t operation,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString sourceFile,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] sourceAttributes,
      [In]int numSourceAttributes,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString targetFile,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] targetAttributes,
      [In]int numTargetAttributes,
      [In]uint ipAddress,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userID,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appPath,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appURL,
      [In]bool performObligation,
      [In]CETYPE.CENoiseLevel_t noiseLevel,
      [Out]out IntPtr enforcement_ob,
      [Out]out CETYPE.CEResponse_t enforcement_result,
      [Out]out int numEnforcements,
      [In] int timeout_in_millisec);
    
    [DllImport("cesdk32.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckMsgAttachment(
      [In]IntPtr handle,
      [In]CETYPE.CEAction_t operation,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString sourceFile,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] sourceAttributes,
      [In]int numSourceAttributes,
      [In]int numRecipients,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] recipients,
      [In]uint ipAddress,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userID,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] userAttributes,
      [In]int numUserAttributes,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appPath,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appURL,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] appAttributes,
      [In]int numAppAttributes,
      [In]bool performObligation,
      [In]CETYPE.CENoiseLevel_t noiseLevel,
      [Out]out IntPtr enforcement_ob,
      [Out]out CETYPE.CEResponse_t enforcement_result,
      [Out]out int numEnforcements,
      [In] int timeout_in_millisec);



    [DllImport("cesdk32.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckResources(
      [In]IntPtr handle,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString operation,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString sourceName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString sourceType,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] sourceAttributes,
      [In]int numSourceAttributes,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString targetName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString targetType,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] targetAttributes,
      [In]int numTargetAttributes,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userID,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] userAttributes,
      [In]int numUserAttributes,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appPath,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appURL,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] appAttributes,
      [In]int numAppAttributes,
      [In]int numRecipients,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] recipients,
      [In]uint ipAddress,
      [In]bool performObligation,
      [In]CETYPE.CENoiseLevel_t noiseLevel,
      [Out]out IntPtr enforcement_ob,
      [Out]out CETYPE.CEResponse_t enforcement_result,
      [Out]out int numEnforcements,
      [In] int timeout_in_millisec);

    [DllImport("cesdk32.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckResourcesEx(
      [In]IntPtr handle,
      [In]CERequest[] requests,
      [In]int numRequests,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString additionalPQL,
      [In]bool ignoreBuiltinPolicies,
      [In]uint ipAddress,
      [In, Out]int[] results,
      [In, Out]int[] obligationCount,
      [In, Out]IntPtr[] obligations,
      [In]int timeout_in_millisec);

    [DllImport("cesdk32.dll")]
    public static extern int CSCINVOKE_CEEVALUATE_GetString(
      [In]IntPtr ptr,
      [In]int index,
      [Out]out IntPtr strPtr);

    [DllImport("cesdk32.dll")]
    public static extern CETYPE.CEResult_t
      CSCINVOKE_CEEVALUATE_FreeStringArray(
        [In]IntPtr ptr,
        [In]int num);

    [DllImport("cesdk32.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CELOGGING_LogObligationData(
      [In]IntPtr handle,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString logIdentifier,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString assistantName,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] attributes,
      [In]int numAttributes);

  }

  //Declare CE SDK C API for 64-bit cesdk.dll
  public class CESDKAPI_Signature64
  {
    [DllImport("cesdk.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CECONN_Initialize(
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString binaryPath,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userID,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString pdpHost,
      [Out]out IntPtr connectHandle,
      [In]int timeout_in_millisec);

    [DllImport("cesdk.dll")]
    public static extern CETYPE.CEResult_t CECONN_Close(
      [In]IntPtr handle,
      [In]int timeout_in_millisec);

    [DllImport("cesdk.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckPortal(
      [In]IntPtr handle,
      [In]CETYPE.CEAction_t operation,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString sourceURL,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] sourceAttributes,
      [In]int numSourceAttributes,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString targetURL,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] targetAttributes,
      [In]int numTargetAttributes,
      [In]uint ipAddress,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userID,
      [In]bool performObligation,
      [In]CETYPE.CENoiseLevel_t noiseLevel,
      [Out]out IntPtr enforcement_ob,
      [Out]out CETYPE.CEResponse_t enforcement_result,
      [Out]out int numEnforcements,
      [In] int timeout_in_millisec);

    [DllImport("cesdk.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckFile(
      [In]IntPtr handle,
      [In]CETYPE.CEAction_t operation,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString sourceFile,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] sourceAttributes,
      [In]int numSourceAttributes,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString targetFile,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] targetAttributes,
      [In]int numTargetAttributes,
      [In]uint ipAddress,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userID,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appPath,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appURL,
      [In]bool performObligation,
      [In]CETYPE.CENoiseLevel_t noiseLevel,
      [Out]out IntPtr enforcement_ob,
      [Out]out CETYPE.CEResponse_t enforcement_result,
      [Out]out int numEnforcements,
      [In] int timeout_in_millisec);

    [DllImport("cesdk.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckMsgAttachment(
      [In]IntPtr handle,
      [In]CETYPE.CEAction_t operation,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString sourceFile,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] sourceAttributes,
      [In]int numSourceAttributes,
      [In]int numRecipients,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] recipients,
      [In]uint ipAddress,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userID,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] userAttributes,
      [In]int numUserAttributes,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appPath,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appURL,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] appAttributes,
      [In]int numAppAttributes,
      [In]bool performObligation,
      [In]CETYPE.CENoiseLevel_t noiseLevel,
      [Out]out IntPtr enforcement_ob,
      [Out]out CETYPE.CEResponse_t enforcement_result,
      [Out]out int numEnforcements,
      [In] int timeout_in_millisec);



    [DllImport("cesdk.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckResources(
      [In]IntPtr handle,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString operation,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString sourceName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString sourceType,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] sourceAttributes,
      [In]int numSourceAttributes,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString targetName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString targetType,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] targetAttributes,
      [In]int numTargetAttributes,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString userID,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] userAttributes,
      [In]int numUserAttributes,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appName,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appPath,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString appURL,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] appAttributes,
      [In]int numAppAttributes,
      [In]int numRecipients,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] recipients,
      [In]uint ipAddress,
      [In]bool performObligation,
      [In]CETYPE.CENoiseLevel_t noiseLevel,
      [Out]out IntPtr enforcement_ob,
      [Out]out CETYPE.CEResponse_t enforcement_result,
      [Out]out int numEnforcements,
      [In] int timeout_in_millisec);

    [DllImport("cesdk.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckResourcesEx(
      [In]IntPtr handle,
      [In]CERequest[] requests,
      [In]int numRequests,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString additionalPQL,
      [In]bool ignoreBuiltinPolicies,
      [In]uint ipAddress,
      [In, Out]int[] results,
      [In, Out]int[] obligationCount,
      [In, Out]IntPtr[] obligations,
      [In]int timeout_in_millisec);

    [DllImport("cesdk.dll")]
    public static extern int CSCINVOKE_CEEVALUATE_GetString(
      [In]IntPtr ptr,
      [In]int index,
      [Out]out IntPtr strPtr);

    [DllImport("cesdk.dll")]
    public static extern CETYPE.CEResult_t
      CSCINVOKE_CEEVALUATE_FreeStringArray(
        [In]IntPtr ptr,
        [In]int num);

    [DllImport("cesdk.dll")]
    public static extern CETYPE.CEResult_t CSCINVOKE_CELOGGING_LogObligationData(
      [In]IntPtr handle,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString logIdentifier,
      [In, MarshalAs(UnmanagedType.LPStruct)]CEString assistantName,
      [In, MarshalAs(UnmanagedType.LPArray,
                     ArraySubType = UnmanagedType.LPTStr)]string[] attributes,
      [In]int numAttributes);
  }

  public class CESDKAPI_Signature
  {
    [DllImport("kernel32.dll")]
    private static extern IntPtr LoadLibrary(string lpFileName);
    [DllImport("kernel32.dll")]
    private static extern IntPtr GetModuleHandle(string lpFileName);
    [DllImport("kernel32.dll")]
    private static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);
    [DllImport("kernel32.dll")]
    private static extern bool IsWow64Process(IntPtr hProcess, ref bool bIsWow64);
    [DllImport("kernel32.dll")]
    private static extern IntPtr GetCurrentProcess();

    private static int KEY_READ = 0x20019;
    private static int KEY_WOW64_64KEY = 0x100;
    private static UIntPtr HKEY_LOCAL_MACHINE = new UIntPtr(0x80000002u);
    private static UIntPtr HKEY_CURRENT_USER = new UIntPtr(0x80000001u);
    [DllImport("advapi32.dll", CharSet = CharSet.Unicode, EntryPoint = "RegQueryValueExW", SetLastError = true)]
    private static extern int RegQueryValueEx(
      UIntPtr hKey,
      string lpValueName,
      int lpReserved,
      out uint lpType,
      System.Text.StringBuilder lpData,
      //ref string lpData,
      ref uint lpcbData);

    [DllImport("advapi32.dll", CharSet = CharSet.Unicode, EntryPoint = "RegOpenKeyExW", SetLastError = true)]
    private static extern int RegOpenKeyEx(
      UIntPtr hKey,
      string subKey,
      int ulOptions,
      int samDesired,
      out UIntPtr hResult);

    [DllImport("advapi32.dll", CharSet = CharSet.Auto)]
    private static extern int RegCloseKey(UIntPtr hKey);

    private static bool bIs64BitCESDK;


    public static CETYPE.CEResult_t CSCINVOKE_CECONN_Initialize(
      CEString appName,
      CEString binaryPath,
      CEString userName,
      CEString userID,
      CEString pdpHost,
      out IntPtr connectHandle,
      int timeout_in_millisec)
    {
      if (bIs64BitCESDK)
        return CESDKAPI_Signature64.CSCINVOKE_CECONN_Initialize(appName, binaryPath, userName, userID, pdpHost,out connectHandle, timeout_in_millisec);
      else
        return CESDKAPI_Signature32.CSCINVOKE_CECONN_Initialize(appName, binaryPath, userName, userID, pdpHost,out connectHandle, timeout_in_millisec);
    }

    public static CETYPE.CEResult_t CECONN_Close(
      IntPtr handle,
      int timeout_in_millisec)
    {
      if (bIs64BitCESDK)
        return CESDKAPI_Signature64.CECONN_Close(handle, timeout_in_millisec);
      else
        return CESDKAPI_Signature32.CECONN_Close(handle, timeout_in_millisec);
        
    }

    public static CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckPortal(
      IntPtr handle,
      CETYPE.CEAction_t operation,
      CEString sourceURL,
      string[] sourceAttributes,
      int numSourceAttributes,
      CEString targetURL,
      string[] targetAttributes,
      int numTargetAttributes,
      uint ipAddress,
      CEString userName,
      CEString userID,
      bool performObligation,
      CETYPE.CENoiseLevel_t noiseLevel,
      out IntPtr enforcement_ob,
      out CETYPE.CEResponse_t enforcement_result,
      out int numEnforcements,
      int timeout_in_millisec)
    {
      if (bIs64BitCESDK)
        return CESDKAPI_Signature64.CSCINVOKE_CEEVALUATE_CheckPortal(handle,
                                                                     operation, sourceURL,
                                                                     sourceAttributes,
                                                                     numSourceAttributes,
                                                                     targetURL,
                                                                     targetAttributes,
                                                                     numTargetAttributes,
                                                                     ipAddress, userName,
                                                                     userID,
                                                                     performObligation,
                                                                     noiseLevel,
                                                                     out enforcement_ob,
                                                                     out enforcement_result,
                                                                     out numEnforcements,
                                                                     timeout_in_millisec);
      else
        return CESDKAPI_Signature32.CSCINVOKE_CEEVALUATE_CheckPortal(handle,
                                                                     operation, sourceURL,
                                                                     sourceAttributes,
                                                                     numSourceAttributes,
                                                                     targetURL,
                                                                     targetAttributes,
                                                                     numTargetAttributes,
                                                                     ipAddress, userName,
                                                                     userID,
                                                                     performObligation,
                                                                     noiseLevel,
                                                                     out enforcement_ob,
                                                                     out enforcement_result,
                                                                     out numEnforcements,
                                                                     timeout_in_millisec);
    }

    public static CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckFile(IntPtr handle,
                                                                   CETYPE.CEAction_t operation,
                                                                   CEString sourceFile,
                                                                   string[] sourceAttributes,
                                                                   int numSourceAttributes,
                                                                   CEString targetFile,
                                                                   string[] targetAttributes,
                                                                   int numTargetAttributes,
                                                                   uint ipAddress,
                                                                   CEString userName,
                                                                   CEString userID,
                                                                   CEString appName,
                                                                   CEString appPath,
                                                                   CEString appURL,
                                                                   bool performObligation,
                                                                   CETYPE.CENoiseLevel_t noiseLevel,
                                                                   out IntPtr enforcement_ob,
                                                                   out CETYPE.CEResponse_t enforcement_result,
                                                                   out int numEnforcements,
                                                                   int timeout_in_millisec)
    {
      if (bIs64BitCESDK)
        return CESDKAPI_Signature64.CSCINVOKE_CEEVALUATE_CheckFile(handle, operation,
                                                                   sourceFile, sourceAttributes, numSourceAttributes,
                                                                   targetFile, targetAttributes, numTargetAttributes,
                                                                   ipAddress, userName, userID, appName, appPath, appURL, performObligation,
                                                                   noiseLevel, out enforcement_ob, out enforcement_result,
                                                                   out numEnforcements, timeout_in_millisec);
      else
        return CESDKAPI_Signature32.CSCINVOKE_CEEVALUATE_CheckFile(handle, operation,
                                                                   sourceFile, sourceAttributes, numSourceAttributes,
                                                                   targetFile, targetAttributes, numTargetAttributes,
                                                                   ipAddress, userName, userID, appName, appPath, appURL, performObligation,
                                                                   noiseLevel, out enforcement_ob, out enforcement_result,
                                                                   out numEnforcements, timeout_in_millisec);
    }

    public static CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckMsgAttachment(IntPtr handle,
                                                                            CETYPE.CEAction_t operation,
                                                                            CEString sourceFile,
                                                                            string[] sourceAttributes,
                                                                            int numSourceAttributes,
                                                                            int numRecipients,
                                                                            string[] recipients,
                                                                            uint ipAddress,
                                                                            CEString userName,
                                                                            CEString userID,
                                                                            string[] userAttributes,
                                                                            int numUserAttributes,
                                                                            CEString appName,
                                                                            CEString appPath,
                                                                            CEString appURL,
                                                                            string[] appAttributes,
                                                                            int numAppAttributes,
                                                                            bool performObligation,
                                                                            CETYPE.CENoiseLevel_t noiseLevel,
                                                                            out IntPtr enforcement_ob,
                                                                            out CETYPE.CEResponse_t enforcement_result,
                                                                            out int numEnforcements,
                                                                            int timeout_in_millisec)
    {
      if (bIs64BitCESDK)
        return CESDKAPI_Signature64.CSCINVOKE_CEEVALUATE_CheckMsgAttachment(handle, operation,
                                                                            sourceFile, sourceAttributes, numSourceAttributes, numRecipients,
                                                                            recipients, ipAddress, userName, userID, userAttributes, numUserAttributes,
                                                                            appName, appPath, appURL, appAttributes, numAppAttributes, performObligation,
                                                                            noiseLevel, out enforcement_ob, out enforcement_result, out numEnforcements, timeout_in_millisec);
      else
        return CESDKAPI_Signature32.CSCINVOKE_CEEVALUATE_CheckMsgAttachment(handle, operation,
                                                                            sourceFile, sourceAttributes, numSourceAttributes, numRecipients,
                                                                            recipients, ipAddress, userName, userID, userAttributes, numUserAttributes,
                                                                            appName, appPath, appURL, appAttributes, numAppAttributes, performObligation,
                                                                            noiseLevel, out enforcement_ob, out enforcement_result, out numEnforcements, timeout_in_millisec);
    }

    public static CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckResources(IntPtr handle,
                                                                        CEString operation,
                                                                        CEString sourceName,
                                                                        CEString sourceType,
                                                                        string[] sourceAttributes,
                                                                        int numSourceAttributes,
                                                                        CEString targetName,
                                                                        CEString targetType,
                                                                        string[] targetAttributes,
                                                                        int numTargetAttributes,
                                                                        CEString userName,
                                                                        CEString userID,
                                                                        string[] userAttributes,
                                                                        int numUserAttributes,
                                                                        CEString appName,
                                                                        CEString appPath,
                                                                        CEString appURL,
                                                                        string[] appAttributes,
                                                                        int numAppAttributes,
                                                                        int numRecipients,
                                                                        string[] recipients,
                                                                        uint ipAddress,
                                                                        bool performObligation,
                                                                        CETYPE.CENoiseLevel_t noiseLevel,
                                                                        out IntPtr enforcement_ob,
                                                                        out CETYPE.CEResponse_t enforcement_result,
                                                                        out int numEnforcements,
                                                                        int timeout_in_millisec)
    {
      if (bIs64BitCESDK)
        return CESDKAPI_Signature64.CSCINVOKE_CEEVALUATE_CheckResources(handle, operation,
                                                                        sourceName, sourceType, sourceAttributes, numSourceAttributes,
                                                                        targetName, targetType, targetAttributes, numTargetAttributes,
                                                                        userName, userID, userAttributes, numUserAttributes,
                                                                        appName, appPath, appURL, appAttributes, numAppAttributes,
                                                                        numRecipients, recipients, ipAddress, performObligation, noiseLevel,
                                                                        out enforcement_ob, out enforcement_result,out numEnforcements, timeout_in_millisec);
      else
        return CESDKAPI_Signature32.CSCINVOKE_CEEVALUATE_CheckResources(handle, operation,
                                                                        sourceName, sourceType, sourceAttributes, numSourceAttributes,
                                                                        targetName, targetType, targetAttributes, numTargetAttributes,
                                                                        userName, userID, userAttributes, numUserAttributes,
                                                                        appName, appPath, appURL, appAttributes, numAppAttributes,
                                                                        numRecipients, recipients, ipAddress, performObligation, noiseLevel,
                                                                        out enforcement_ob, out enforcement_result, out numEnforcements, timeout_in_millisec);
    }

    public static CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_CheckResourcesEx(IntPtr handle,
                                                                          CERequest[] requests,
                                                                          CEString additionalPQL,
                                                                          bool ignoreBuiltinPolicies,
                                                                          uint ipAddress,
                                                                          int[] results,
                                                                          int[] obligationCounts,
                                                                          IntPtr[] obligations,
                                                                          int timeout_in_millisec)
    {  
      if (bIs64BitCESDK) {
        return CESDKAPI_Signature64.CSCINVOKE_CEEVALUATE_CheckResourcesEx(handle, requests, requests.Length, additionalPQL, ignoreBuiltinPolicies,
                                                                          ipAddress, results, obligationCounts, obligations, timeout_in_millisec);
      } else {
        return CESDKAPI_Signature32.CSCINVOKE_CEEVALUATE_CheckResourcesEx(handle, requests, requests.Length, additionalPQL, ignoreBuiltinPolicies,
                                                                          ipAddress, results, obligationCounts, obligations, timeout_in_millisec);
      }
    }

    public static int CSCINVOKE_CEEVALUATE_GetString(IntPtr ptr,
                                                     int index,
                                                     out IntPtr strPtr)
    {
      if (bIs64BitCESDK)
        return CESDKAPI_Signature64.CSCINVOKE_CEEVALUATE_GetString(ptr, index, out strPtr);
      else
        return CESDKAPI_Signature32.CSCINVOKE_CEEVALUATE_GetString(ptr, index, out strPtr);
    }

    public static CETYPE.CEResult_t CSCINVOKE_CEEVALUATE_FreeStringArray(IntPtr ptr,
                                                                         int num)
    {
      if (bIs64BitCESDK)
        return CESDKAPI_Signature64.CSCINVOKE_CEEVALUATE_FreeStringArray(ptr, num);
      else
        return CESDKAPI_Signature32.CSCINVOKE_CEEVALUATE_FreeStringArray(ptr, num);
    }

    public static CETYPE.CEResult_t CSCINVOKE_CELOGGING_LogObligationData(IntPtr handle,
                                                                          CEString logIdentifier,
                                                                          CEString assistantName,
                                                                          string[] attributes,
                                                                          int numAttributes)
    {
      if (bIs64BitCESDK)
        return CESDKAPI_Signature64.CSCINVOKE_CELOGGING_LogObligationData(handle, logIdentifier, assistantName, attributes, numAttributes);
      else
        return CESDKAPI_Signature32.CSCINVOKE_CELOGGING_LogObligationData(handle, logIdentifier, assistantName, attributes, numAttributes);
    }

    public static bool Is64BitOS()
    {
      if (IntPtr.Size == 8)
        return true;
      else
      {
        IntPtr hModule = GetModuleHandle("kernel32.dll");
        if (hModule == IntPtr.Zero)
          return false;

        IntPtr hProc = GetProcAddress(hModule, "IsWow64Process");
        if (hProc == IntPtr.Zero)
          return false;

        bool bIsWow64 = false;
        if (IsWow64Process(GetCurrentProcess(), ref bIsWow64))
          return bIsWow64;

        return false;
      }
    }

    public static bool GetCommonLibrariesBin(ref string strBin)
    {
      bool b64bitOS = Is64BitOS();
      int samDesired = KEY_READ;
      if (IntPtr.Size == 4 && b64bitOS == true)
        samDesired |= KEY_WOW64_64KEY;

      if (ReadRegKey(HKEY_LOCAL_MACHINE, "Software\\Nextlabs\\CommonLibraries\\", "InstallDir", ref strBin, samDesired) == false)
        return false;


      if (strBin.EndsWith("\\") == false && strBin.EndsWith("/") == false)
        strBin += "\\";
      if (IntPtr.Size == 8)
        strBin += "bin64\\";
      else
        strBin += "bin32\\";
      return true;
    }

    public static bool ReadRegKey(UIntPtr rootkey, string keypath, string valueName, ref string keyvalue, int samDesired)
    {
      UIntPtr hKey;
      if (RegOpenKeyEx(rootkey, keypath, 0, samDesired, out hKey) == 0)
      {
        uint size = 1024;
        uint type;
        StringBuilder keyBuffer = new StringBuilder(1024);
        if (RegQueryValueEx(hKey, valueName, 0, out type, keyBuffer, ref size) == 0)
          keyvalue = keyBuffer.ToString();

        RegCloseKey(hKey);
        return true;
      }
      return false;
    }
    
    static CESDKAPI_Signature()
    {
      bIs64BitCESDK = (IntPtr.Size==8);
      string strCommonLibBin=null;
      if (GetCommonLibrariesBin(ref strCommonLibBin) == true)
      {
        if (bIs64BitCESDK)
          strCommonLibBin += "cesdk.dll";
        else
          strCommonLibBin += "cesdk32.dll";
        IntPtr hModule=LoadLibrary(strCommonLibBin);
        if (hModule == IntPtr.Zero)
        {
          Trace.WriteLine("Fail to load CESDK with the path:" + strCommonLibBin);
        }
      }
    }
  }
  //Define CE SDK .NET 
  public class CESDKAPI
  {
    /* ------------------------------------------------------------------------
     * CECONN_Initialize()
     *
     * Initializes the connection between the client to the Policy Decision
     * Point Server.
     * 
     * Arguments : 
     *             app (INPUT): the application assoicate with the client PEP
     *             user: to identify a user in the application
     *             pdpHost (INPUT): Name of PDP host. If it is NULL, it means the
     *                              local machine.
     *             timeout_in_millisec (INPUT): Desirable timeout in milliseconds 
     *             for this RPC
     *             connectHandle (OUTPUT): connection handle for subsequent call
     * Return: return CETYPE.CE_RESULT_SUCCESS if the call succeeds.
     * ------------------------------------------------------------------------
     */
    public static CETYPE.CEResult_t CECONN_Initialize(CETYPE.CEApplication app,
                                                      CETYPE.CEUser user,
                                                      string pdpHost,
                                                      out IntPtr connectHandle,
                                                      int timeout_in_millisec)
    {
      CEString ces_appName = new CEString(app.appName);
      CEString ces_binaryPath = new CEString(app.appPath);
      CEString ces_userName = new CEString(user.userName);
      CEString ces_userID = new CEString(user.userID);
      CEString ces_pdpHost = new CEString(pdpHost);

      Thread.BeginThreadAffinity();
      CETYPE.CEResult_t result = CESDKAPI_Signature.CSCINVOKE_CECONN_Initialize(
        ces_appName,
        ces_binaryPath,
        ces_userName,
        ces_userID,
        ces_pdpHost,
        out connectHandle,
        timeout_in_millisec);
      Thread.EndThreadAffinity();
      return result;
    }

    /* ------------------------------------------------------------------------
     * CECONN_Close()
     *
     * Close the connection between the client and the Policy Decision
     * Point Server.
     * 
     * Arguments : handle (INPUT): connection handle from the CONN_initialize API
     *             timeout_in_millisec (INPUT): Desirable timeout in milliseconds 
     *             for this RPC
     *             
     * Return: return CETYPE.CE_RESULT_SUCCESS if the call succeeds.
     * ------------------------------------------------------------------------
     */
    public static CETYPE.CEResult_t CECONN_Close(IntPtr handle,
                                                 int timeout_in_millisec)
    {
      Thread.BeginThreadAffinity();
      CETYPE.CEResult_t result = CESDKAPI_Signature.CECONN_Close(handle,
                                                                 timeout_in_millisec);
      Thread.EndThreadAffinity();
      return result;
    }

    /* ------------------------------------------------------------------------
     * CEEVALUATE_CheckPortal()
     *
     * Ask the Policy Decision Point Server to evaluate the operation.
     *
     * Arguments : 
     * handle (INPUT): Handle from the CONN_Initialize()
     * Operation (INPUT): Operation on the file
     * sourceURL (INPUT): the URL to the source resource
     * targetURL (INPUT): the URL to the target resource
     * performObligation (INPUT): Perform the obligation defined by the policy 
     *                            (e.g. logging / email)
     * sourceAttributes	(INPUT): Associate attributes of the source. This is
     *   a string array in the order of "key-1""value-1""key-2""value-2"...
     * targetAttributes	(INPUT): Associate attributes of the target. This is
     *   a string array in the order of "key-1""value-1""key-2""value-2"...
     * noiseLevel	(INPUT): Desirable noise level to be used for this evaluation
     * ipAddress (INPUT): For Sharepointe, the ip address of client machine
     * this evaluation
     * user (INPUT): to identify the user who accesses the URL
     * enforcement_ob (OUTPUT): the resulted enforcement obligation  from 
     *   the policy decision point server. This is a string array in the order of 
     *   "key-1""value-1""key-2""value-2"...
     * enforcement_result (OUTPUT): the resulted enforcement integer decision.
     * timeout_in_millisec (INPUT): Desirable timeout in milliseconds for 
     *                              this RPC
     *
     * Return: return CETYPE.CE_RESULT_SUCCESS if the call succeeds.
     * ------------------------------------------------------------------------
     */
    public static CETYPE.CEResult_t CEEVALUATE_CheckPortal(IntPtr handle,
                                                           CETYPE.CEAction_t operation,
                                                           string sourceURL,
                                                           ref string[] sourceAttributes,
                                                           string targetURL,
                                                           ref string[] targetAttributes,
                                                           uint ipAddress,
                                                           CETYPE.CEUser user,
                                                           bool performObligation,
                                                           CETYPE.CENoiseLevel_t noiseLevel,
                                                           out string[] enforcement_obligation,
                                                           out CETYPE.CEResponse_t enforcement_result,
                                                           int timeout_in_millisec)
    {
      CEString ces_sourceURL = new CEString(sourceURL);
      CEString ces_targetURL = new CEString(targetURL);
      CEString ces_userName = new CEString(user.userName);
      CEString ces_userID = new CEString(user.userID);

      IntPtr enforcement_ob;
      int numEnforcements;

      CETYPE.CEResult_t result;
      Thread.BeginThreadAffinity();
      result = CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_CheckPortal(handle,
                                                                   operation,
                                                                   ces_sourceURL,
                                                                   sourceAttributes,
                                                                   (sourceAttributes != null) ? sourceAttributes.Length : 0,
                                                                   ces_targetURL,
                                                                   targetAttributes,
                                                                   (targetAttributes != null) ? targetAttributes.Length : 0,
                                                                   ipAddress,
                                                                   ces_userName,
                                                                   ces_userID,
                                                                   performObligation,
                                                                   noiseLevel,
                                                                   out enforcement_ob,
                                                                   out enforcement_result,
                                                                   out numEnforcements,
                                                                   timeout_in_millisec);
      Thread.EndThreadAffinity();

      if (result != CETYPE.CEResult_t.CE_RESULT_SUCCESS)
      {
        numEnforcements = 0;
        enforcement_obligation = null;
        return result;
      }

      enforcement_obligation = new string[numEnforcements];
      int strLen;
      IntPtr strPtr;
      for (int i = 0; i < numEnforcements; i++)
      {
        strLen = CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_GetString(enforcement_ob,
                                                                   i, out strPtr);
        if(strLen != 0)
          enforcement_obligation[i] = Marshal.PtrToStringAuto(strPtr, strLen);
        else

          enforcement_obligation[i] = null;
      }
      CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_FreeStringArray(enforcement_ob,
                                                              numEnforcements);
      return result;
    }

    /* ------------------------------------------------------------------------
     * CEEVALUATE_CheckFile()
     *
     * Ask the Policy Decision Point Server to evaluate the operation on file.
     *
     * Arguments : 
     * handle (INPUT): Handle from the CONN_Initialize()
     * Operation (INPUT): Operation on the file
     * sourceFile (INPUT): the source file.
     * targetFile (INPUT): the target file.
     * performObligation (INPUT): Perform the obligation defined by the policy 
     *                            (e.g. logging / email)
     * sourceAttributes	(INPUT): Associate attributes of the source. This is
     *   a string array in the order of "key-1""value-1""key-2""value-2"...
     * targetAttributes	(INPUT): Associate attributes of the target. This is
     *   a string array in the order of "key-1""value-1""key-2""value-2"...
     * noiseLevel	(INPUT): Desirable noise level to be used for this evaluation
     * ipAddress (INPUT): For Sharepointe, the ip address of client machine
     * this evaluation
     * user (INPUT): to identify the user who accesses the files.
     * app (INPUT): to identify the application that does the operation. 
     * enforcement_ob (OUTPUT): the resulted enforcement obligation  from 
     *   the policy decision point server. This is a string array in the order of 
     *   "key-1""value-1""key-2""value-2"...
     * enforcement_result (OUTPUT): the resulted enforcement integer decision.
     * timeout_in_millisec (INPUT): Desirable timeout in milliseconds for 
     *                              this RPC
     *
     * Return: return CETYPE.CE_RESULT_SUCCESS if the call succeeds.
     * ------------------------------------------------------------------------
     */
    public static CETYPE.CEResult_t CEEVALUATE_CheckFile(IntPtr handle,
                                                         CETYPE.CEAction_t operation,
                                                         string sourceFile,
                                                         ref string[] sourceAttributes,
                                                         string targetFile,
                                                         ref string[] targetAttributes,
                                                         uint ipAddress,
                                                         CETYPE.CEUser user,
                                                         CETYPE.CEApplication app,
                                                         bool performObligation,
                                                         CETYPE.CENoiseLevel_t noiseLevel,
                                                         out string[] enforcement_obligation,
                                                         out CETYPE.CEResponse_t enforcement_result,
                                                         int timeout_in_millisec)
    {
      CEString ces_source = new CEString(sourceFile);
      CEString ces_target = new CEString(targetFile);

      CEString ces_userName = new CEString(user.userName);
      CEString ces_userID = new CEString(user.userID);

      CEString ces_appName = new CEString(app.appName);
      CEString ces_appPath = new CEString(app.appPath);
      CEString ces_appURL = new CEString(app.appURL);

      IntPtr enforcement_ob;
      int numEnforcements;

      CETYPE.CEResult_t result;
      Thread.BeginThreadAffinity();
      result = CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_CheckFile(handle,
                                                                 operation,
                                                                 ces_source,
                                                                 sourceAttributes,
                                                                 (sourceAttributes != null) ? sourceAttributes.Length : 0,
                                                                 ces_target,
                                                                 targetAttributes,
                                                                 (targetAttributes != null) ? targetAttributes.Length : 0,
                                                                 ipAddress,
                                                                 ces_userName,
                                                                 ces_userID,
                                                                 ces_appName,
                                                                 ces_appPath,
                                                                 ces_appURL,
                                                                 performObligation,
                                                                 noiseLevel,
                                                                 out enforcement_ob,
                                                                 out enforcement_result,
                                                                 out numEnforcements,
                                                                 timeout_in_millisec);
      Thread.EndThreadAffinity();

      if (result != CETYPE.CEResult_t.CE_RESULT_SUCCESS)
      {
        numEnforcements = 0;
        enforcement_obligation = null;
        return result;
      }

      enforcement_obligation = new string[numEnforcements];
      int strLen;
      IntPtr strPtr;
      for (int i = 0; i < numEnforcements; i++)
      {
        strLen = CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_GetString(enforcement_ob,
                                                                   i, out strPtr);

        if (strLen != 0)

          enforcement_obligation[i] = Marshal.PtrToStringAuto(strPtr, strLen);

        else

          enforcement_obligation[i] = null;
      }
      CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_FreeStringArray(enforcement_ob,
                                                              numEnforcements);
      return result;
    }

    /* ------------------------------------------------------------------------
     * CEEVALUATE_CheckMessageAttachment()
     *
     * Ask the Policy Decision Point Server to evaluate the operation on sending
     * a message with attachment.
     *
     * Arguments : 
     * handle (INPUT): Handle from the CONN_Initialize()
     * Operation (INPUT): Operation on the file
     * sourceFile (INPUT): the source file.
     * performObligation (INPUT): Perform the obligation defined by the policy 
     *                            (e.g. logging / email)
     * sourceAttributes	(INPUT): Associate attributes of the source. This is
     *   a string array in the order of "key-1""value-1""key-2""value-2"...
     * recipients (INPUT): the string array of the message recipients
     * noiseLevel	(INPUT): Desirable noise level to be used for this evaluation
     * ipAddress (INPUT): For Sharepointe, the ip address of client machine
     * this evaluation
     * user (INPUT): to identify the user who accesses the files.
     * userAttributes	(INPUT): Associate attributes of the user. This is
     *   a string array in the order of "key-1""value-1""key-2""value-2"...
     * app (INPUT): to identify the application that does the operation. 
     * appAttributes	(INPUT): Associate attributes of the application. This is
     *   a string array in the order of "key-1""value-1""key-2""value-2"...
     * enforcement_ob (OUTPUT): the resulted enforcement obligation  from 
     *   the policy decision point server. This is a string array in the order of 
     *   "key-1""value-1""key-2""value-2"...
     * enforcement_result (OUTPUT): the resulted enforcement integer decision.
     * timeout_in_millisec (INPUT): Desirable timeout in milliseconds for 
     *                              this RPC
     *
     * Return: return CETYPE.CE_RESULT_SUCCESS if the call succeeds.
     * ------------------------------------------------------------------------
     */

    public static CETYPE.CEResult_t CEEVALUATE_CheckMessageAttachment(IntPtr handle,
                                                                      CETYPE.CEAction_t operation,
                                                                      string sourceFile,
                                                                      ref string[] sourceAttributes,
                                                                      ref string[] recipients,
                                                                      uint ipAddress,
                                                                      CETYPE.CEUser user,
                                                                      ref string[] userAttributes,
                                                                      CETYPE.CEApplication app,
                                                                      ref string[] appAttributes,
                                                                      bool performObligation,
                                                                      CETYPE.CENoiseLevel_t noiseLevel,
                                                                      out string[] enforcement_obligation,
                                                                      out CETYPE.CEResponse_t enforcement_result,
                                                                      int timeout_in_millisec)
    {
      CEString ces_source = new CEString(sourceFile);

      CEString ces_userName = new CEString(user.userName);
      CEString ces_userID = new CEString(user.userID);

      CEString ces_appName = new CEString(app.appName);
      CEString ces_appPath = new CEString(app.appPath);
      CEString ces_appURL = new CEString(app.appURL);

      IntPtr enforcement_ob;
      int numEnforcements;

      CETYPE.CEResult_t result;
      Thread.BeginThreadAffinity();
      result = CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_CheckMsgAttachment(handle,
                                                                          operation,
                                                                          ces_source,
                                                                          sourceAttributes,
                                                                          (sourceAttributes != null) ? sourceAttributes.Length : 0,
                                                                          (recipients != null) ? recipients.Length : 0,
                                                                          recipients,
                                                                          ipAddress,
                                                                          ces_userName,
                                                                          ces_userID,
                                                                          userAttributes,
                                                                          (userAttributes != null) ? userAttributes.Length : 0,
                                                                          ces_appName,
                                                                          ces_appPath,
                                                                          ces_appURL,
                                                                          appAttributes,
                                                                          (appAttributes != null) ? appAttributes.Length : 0,
                                                                          performObligation,
                                                                          noiseLevel,
                                                                          out enforcement_ob,
                                                                          out enforcement_result,
                                                                          out numEnforcements,
                                                                          timeout_in_millisec);
      Thread.EndThreadAffinity();

      if (result != CETYPE.CEResult_t.CE_RESULT_SUCCESS)
      {
        numEnforcements = 0;
        enforcement_obligation = null;
        return result;
      }

      enforcement_obligation = new string[numEnforcements];
      int strLen;
      IntPtr strPtr;
      for (int i = 0; i < numEnforcements; i++)
      {
        strLen = CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_GetString(enforcement_ob,
                                                                   i, out strPtr);

        if (strLen != 0)

          enforcement_obligation[i] = Marshal.PtrToStringAuto(strPtr, strLen);

        else

          enforcement_obligation[i] = null;
      }
      CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_FreeStringArray(enforcement_ob,
                                                              numEnforcements);
      return result;
    }




    /* ------------------------------------------------------------------------
     * CEEVALUATE_CheckResources()
     *
     * Ask the Policy Controller to evaluate the operation on resources.
     *
     * Arguments : 
     * handle (INPUT): Handle from the CONN_Initialize()
     * Operation (INPUT): Operation on the file
     * source (INPUT): the source resource.
     * sourceAttributes	(INPUT): Associate attributes of the source. This is
     *   a string array in the order of "key-1""value-1""key-2""value-2"...
     * target (INPUT): the target resource.
     * targetAttributes	(INPUT): Associate attributes of the target. This is
     *   a string array in the order of "key-1""value-1""key-2""value-2"...
     * user (INPUT): to identify the user who accesses the files.
     * userAttributes	(INPUT): Associate attributes of the user. This is
     *   a string array in the order of "key-1""value-1""key-2""value-2"...
     * app (INPUT): to identify the application that does the operation. 
     * appAttributes	(INPUT): Associate attributes of the application. This is
     *   a string array in the order of "key-1""value-1""key-2""value-2"...
     * recipients (INPUT): the string array of the recipients for the case of messaging. 
     * ipAddress (INPUT): For Sharepointe, the ip address of client machine
     * performObligation (INPUT): Perform the obligation defined by the policy 
     *                            (e.g. logging / email)
     * noiseLevel	(INPUT): Desirable noise level to be used for this evaluation
     * enforcement_ob (OUTPUT): the resulted enforcement obligation  from 
     *   the policy decision point server. This is a string array in the order of 
     *   "key-1""value-1""key-2""value-2"...
     * enforcement_result (OUTPUT): the resulted enforcement integer decision.
     * timeout_in_millisec (INPUT): Desirable timeout in milliseconds for 
     *                              this RPC
     *
     * Note:
     *   Resource names entered in "source" and "target" are different from 
     *   attributes called "name" in "sourceAttributes" and "targetAttributes".
     *   The former is copied into CE::ID, and is used by Policy Controller as
     *     an internal identification number.
     *   The latter is matched against resource.*.name in PQL.
     *   If you try to match a resource name against resource.*.name, you need to
     *     set "name" in "sourceAttributes" or "targetAttributes" explicitly.
     *
     * Return: return CETYPE.CE_RESULT_SUCCESS if the call succeeds.
     * ------------------------------------------------------------------------
     */

    public static CETYPE.CEResult_t CEEVALUATE_CheckResources(
      IntPtr handle,
      string operation,
      CETYPE.CEResource source,
      ref string[] sourceAttributes,
      CETYPE.CEResource target,
      ref string[] targetAttributes,
      CETYPE.CEUser user,
      ref string[] userAttributes,
      CETYPE.CEApplication app,
      ref string[] appAttributes,
      ref string[] recipients,
      uint ipAddress,
      bool performObligation,
      CETYPE.CENoiseLevel_t noiseLevel,
      out string[] enforcement_obligation,
      out CETYPE.CEResponse_t enforcement_result,
      int timeout_in_millisec)
    {
      CEString ces_operation = new CEString(operation);
            
      CEString ces_source_name = new CEString(source.resourceName);
      CEString ces_source_type = new CEString(source.resourceType);

      CEString ces_target_name = new CEString(target.resourceName);
      CEString ces_target_type = new CEString(target.resourceType);

      CEString ces_userName = new CEString(user.userName);
      CEString ces_userID = new CEString(user.userID);

      CEString ces_appName = new CEString(app.appName);
      CEString ces_appPath = new CEString(app.appPath);
      CEString ces_appURL = new CEString(app.appURL);

      IntPtr enforcement_ob;
      int numEnforcements;

      CETYPE.CEResult_t result;
      Thread.BeginThreadAffinity();
      result = CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_CheckResources(handle,
                                                                      ces_operation,
                                                                      ces_source_name,
                                                                      ces_source_type,
                                                                      sourceAttributes,
                                                                      (sourceAttributes != null) ? sourceAttributes.Length : 0,
                                                                      ces_target_name,
                                                                      ces_target_type,
                                                                      targetAttributes,
                                                                      (targetAttributes != null) ? targetAttributes.Length : 0,
                                                                      ces_userName,
                                                                      ces_userID,
                                                                      userAttributes,
                                                                      (userAttributes != null) ? userAttributes.Length : 0,
                                                                      ces_appName,
                                                                      ces_appPath,
                                                                      ces_appURL,
                                                                      appAttributes,
                                                                      (appAttributes != null) ? appAttributes.Length : 0,
                                                                      (recipients != null) ? recipients.Length : 0,
                                                                      recipients,
                                                                      ipAddress,
                                                                      performObligation,
                                                                      noiseLevel,
                                                                      out enforcement_ob,
                                                                      out enforcement_result,
                                                                      out numEnforcements,
                                                                      timeout_in_millisec);
      Thread.EndThreadAffinity();

      if (result != CETYPE.CEResult_t.CE_RESULT_SUCCESS)
      {
        numEnforcements = 0;
        enforcement_obligation = null;
        return result;
      }

      enforcement_obligation = new string[numEnforcements];
      int strLen;
      IntPtr strPtr;
      for (int i = 0; i < numEnforcements; i++)
      {
        strLen = CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_GetString(enforcement_ob,
                                                                   i, out strPtr);
        if (strLen != 0)
          enforcement_obligation[i] = Marshal.PtrToStringAuto(strPtr, strLen);
        else
          enforcement_obligation[i] = null;
      }
      CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_FreeStringArray(enforcement_ob,
                                                              numEnforcements);
      return result;
    }

    public static CETYPE.CEResult_t CEEVALUATE_CheckResourcesEx(IntPtr handle,
                                                                CETYPE.CERequest[] requests,
                                                                string additionalPQL,
                                                                bool ignoreBuiltinPolicies,
                                                                uint ipAddress,
                                                                out CETYPE.CEEnforcement[] enforcements,
                                                                int timeout_in_millis)
    {
      CEString pql = new CEString(additionalPQL);

      CERequest[] reqs = new CERequest[requests.Length];

      for (int i = 0; i < requests.Length; i++)
      {
        reqs[i] = new CERequest(requests[i]);
      }

      // It's difficult to pass back an array of CEEnforcment objects, because unpacking them is hard, so we'll pass the
      // results back in three pieces
      //
      // * an array of results (allow/deny)
      // * an array of obligation counts (number of obligations for each result)
      // * an array of array of strings, representing the obligations

      int[] results = new int[requests.Length];
      int[] obligationsCounts = new int[requests.Length];
      IntPtr[] allObligations = new IntPtr[requests.Length];

      Thread.BeginThreadAffinity();
      CETYPE.CEResult_t res = CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_CheckResourcesEx(handle, reqs, pql, ignoreBuiltinPolicies, ipAddress,
                                                                                       results, obligationsCounts, allObligations,
                                                                                       timeout_in_millis);
      Thread.EndThreadAffinity();

      for (int i = 0; i < requests.Length; i++)
      {
        reqs[i].Dispose();
      }

      if (res == CETYPE.CEResult_t.CE_RESULT_SUCCESS)
      {
        enforcements = new CETYPE.CEEnforcement[requests.Length];

        for (int i = 0; i < requests.Length; i++)
        {
          string[] obligations = new string[obligationsCounts[i]];
          for (int j = 0; j < obligationsCounts[i]; j++)
          {
            IntPtr ptr;
            int len = CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_GetString(allObligations[i], j, out ptr);
            if (len != 0)
            {
              obligations[j] = Marshal.PtrToStringAuto(ptr, len);
            }
            else
            {
              obligations[j] = null;
            }
          }
          CESDKAPI_Signature.CSCINVOKE_CEEVALUATE_FreeStringArray(allObligations[i], obligationsCounts[i]);

          enforcements[i] = new CETYPE.CEEnforcement(results[i], obligations);
        }
      }
      else
      {
        enforcements = null;
      }

      return res;
    }

    /* ------------------------------------------------------------------------
       /*! CELOGGING_LogObligationData
       *
       * \brief This assistant logging obligation. This function will be called by the Policy Assistant 
       * (or by multiple Policy Assistants).
       * 
       * \param logIdentifier (in): Taken from the obligation information.  Note that this is actually a long integer, 
       * \param obligationName: The name of the obligation (e.g. "CE Encryption Assistant"
       * \param attributes (in): These are unstructured key/value pairs representing information that this particular 
       * Policy Assistant would like presented in the log. Currently, only the first three attributes will be assigned the fields in the log.
       *
       * \return Result of logging.
       *
       * \sa CELOGGING_LogObligationData
       */
    public static CETYPE.CEResult_t CELOGGING_LogObligationData(IntPtr handle,
                                                                string logIdentifier,
                                                                string assistantName,
                                                                ref string[] attributes)
    {
      CEString ces_logIdentifier = new CEString(logIdentifier);

      CEString ces_assistantName = new CEString(assistantName);

      CETYPE.CEResult_t result;
      Thread.BeginThreadAffinity();
      result = CESDKAPI_Signature.CSCINVOKE_CELOGGING_LogObligationData(handle,
                                                                        ces_logIdentifier,
                                                                        ces_assistantName,
                                                                        attributes,
                                                                        (attributes != null) ? attributes.Length : 0);
      Thread.EndThreadAffinity();

      return result;
    }
  }
}

