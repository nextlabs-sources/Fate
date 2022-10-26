using System;
using System.Collections.Generic;
using System.Text;
using NextLabs.CSCInvoke;

namespace CSCInvoke_test
{
  class Test_CSCInvoke
  {
    static void TestCheckResource(IntPtr connectHandle)
    {
        CETYPE.CEResource source = new CETYPE.CEResource("csinvoke_checkresource_source",
                                                        "csinvoke_checkresource_source_type");
        CETYPE.CEResource target = new CETYPE.CEResource(null, null);
        CETYPE.CEUser user = new CETYPE.CEUser("heidiz", null);
        CETYPE.CEApplication app = new CETYPE.CEApplication("Test_CSCInvoke_checkresource", null,
                                                            "http://mail.yahoo.com");
        string[] sourceAttributes = new string[1 * 2];
        string[] targetAttributes = null;
        string[] out_enforcement_obligation;
        CETYPE.CEResponse_t out_enforcement_result;
        CETYPE.CEResult_t result;

        user.userID = "S-1-5-21-668023798-3031861066-1043980994-3455";
        sourceAttributes[0] = CETYPE.CEAttrKey.CE_ATTR_MODIFIED_DATE;
        sourceAttributes[1] = "1234567";
        string[] recipients = new string[1];
        recipients[0] = "hzhou@nextlabs.com";
        string[] userAttributes = new string[2];
        string[] appAttributes = new string[2];
        userAttributes[0] = "user-attr-key";
        userAttributes[1] = "user-attr-value";
        appAttributes[0] = "app-attr-key";
        appAttributes[1] = "app-attr-value";

        result = CESDKAPI.CEEVALUATE_CheckResources(connectHandle,
                                "csinvoke_checkresource_operation", //operation
                                source, 
                                ref sourceAttributes,
                                target, //target
                                ref targetAttributes, //target attributes
                                user,
                                ref userAttributes,
                                app,
                                ref appAttributes,
                                ref recipients,
                                999, //ip address
                                true,
                                CETYPE.CENoiseLevel_t.CE_NOISE_LEVEL_APPLICATION, //noise level
                                out out_enforcement_obligation,
                                out out_enforcement_result,
                                5000 //timeout
                            );
        Console.WriteLine("CEEVALUATE_CheckResource returns: {0}.",
                result);
        if (result == CETYPE.CEResult_t.CE_RESULT_SUCCESS)
        {
            //Verify the result from CEEVALUATE_CheckResource
            Console.WriteLine("CEEVALUATE_CheckResource result: {0} ({1}).",
                               out_enforcement_result,
                               (out_enforcement_result == CETYPE.CEResponse_t.CEAllow) ? "allow" : "deny");

            for (int i = 0; i < out_enforcement_obligation.Length; i++)
            {
                Console.WriteLine("CEEVALUATE_CheckResource: {0} obligation key {1}.",
                i, out_enforcement_obligation[i]);
                i++;
                Console.WriteLine("CEEVALUATE_CheckResource: {0} obligation key {1}.",
                i, out_enforcement_obligation[i]);
            }
            //End: Verify the result from CEEVALUATE_CheckResource
        }

    }

    static void TestCheckResourceEx(IntPtr connectHandle)
    {
      CETYPE.CERequest[] requests = new CETYPE.CERequest[2];

      CETYPE.CENamedAttributes[] named = new CETYPE.CENamedAttributes[2];
      named[0] = new CETYPE.CENamedAttributes("foo", new string[] { "abc", "def",
                                                                    "ghi", "jkl",
                                                                    "mno", "pqrs",
                                                                    "tuv", "wxyz" });

      named[1] = new CETYPE.CENamedAttributes("bar", new string[] {"123", "456"});


      requests[0] = new CETYPE.CERequest("OPEN",
                                         new CETYPE.CEResource("1.txt", "fso"),
                                         new string[] {"key1", "val1", "key2", "val2" },
                                         new CETYPE.CEResource("target1.txt", "fso"),
                                         new string[] {"tkey1", "tval1" },
                                         new CETYPE.CEUser("name1", "id1"),
                                         new string[] {"u1k1", "u1v1"},
                                         new CETYPE.CEApplication("app1", "app1p", "app1url"),
                                         new string[] {"a1k1", "a1v1"},
                                         new string[] {"recip1", "recip2", "recip3"},
                                         named,
                                         true,
                                         CETYPE.CENoiseLevel_t.CE_NOISE_LEVEL_SYSTEM);

      named = new CETYPE.CENamedAttributes[1];

      named[0] = new CETYPE.CENamedAttributes("bar", new string[] {"1234", "5678",
                                                                   "AbCd", "XKCD"});

      requests[1] = new CETYPE.CERequest("DELETE",
                                         new CETYPE.CEResource("2.txt", "fso"),
                                         new string[] {"keya", "vala", "keyb", "valb"},
                                         new CETYPE.CEResource("target2.txt", "fso"),
                                         new string[] {"tkey2", "tval2"},
                                         new CETYPE.CEUser("name2", "id2"),
                                         new string[] {"u2k1", "u2v1"},
                                         new CETYPE.CEApplication("app2", "app2p", "app2url"),
                                         new string[] {"a2k1", "a2v1"},
                                         new string[] { "recip_a", "recip_b", },
                                         named,
                                         true,
                                         CETYPE.CENoiseLevel_t.CE_NOISE_LEVEL_USER_ACTION);

      CETYPE.CEEnforcement[] enfs = new CETYPE.CEEnforcement[requests.Length];

      CETYPE.CEResult_t res = CESDKAPI.CEEVALUATE_CheckResourcesEx(connectHandle,
                                                                   requests,
                                                                   "POLICY X FOR * ON DELETE BY * DO DENY ON DENY DO DISPLAY(\"Oops\")",
                                                                   false,
                                                                   0,
                                                                   out enfs,
                                                                   30000);
      
      if (res == CETYPE.CEResult_t.CE_RESULT_SUCCESS)
      {
        Console.WriteLine("Success!");
        
        foreach(CETYPE.CEEnforcement enf in enfs)
        {
          Console.Write("Result: ");
          Console.WriteLine((int)enf.result);

          foreach (string ob in enf.obligations)
          {
            Console.Write("\t");
            Console.WriteLine(ob);
          }
        }
      }
      else
      {
        Console.Write("Failure: ");
        Console.WriteLine((int)res);
      }

      
    }

    public static void Main()
    {
	    IntPtr connectHandle;
        const int MAX_ATTRIBUTES = 5;
        CETYPE.CEUser user=new CETYPE.CEUser("heidiz", null);
        CETYPE.CEApplication app=new CETYPE.CEApplication("Test_CSCInvoke", null, 
                                                          "http://mail.yahoo.com");

        //Test CECONN_Initialize
        CETYPE.CEResult_t result = CESDKAPI.CECONN_Initialize(app, user, null, out connectHandle, 5000);
        Console.WriteLine("CECONN_Initialize returns: {0}.", result);
        if (result == CETYPE.CEResult_t.CE_RESULT_SUCCESS)
        {
            TestCheckResource(connectHandle);

        TestCheckResourceEx(connectHandle);

            user.userID = "S-1-5-21-668023798-3031861066-1043980994-3455";
            //Test CEEVALUATE_CheckXXX
            string[] enforcement_obligation;
            CETYPE.CEResponse_t enforcement_result;
            string[] sourceAttributes = new string[MAX_ATTRIBUTES * 2];
            string[] targetAttributes = null;
            for (int i = 0; i < MAX_ATTRIBUTES * 2; i++)
            {
                sourceAttributes[i++] = CETYPE.CEAttrKey.CE_ATTR_SP_CREATED_BY;
                sourceAttributes[i] = "dummy value";
            }
            //Test CheckPortal
            result = CESDKAPI.CEEVALUATE_CheckPortal(connectHandle,
                                    CETYPE.CEAction_t.CE_ACTION_WRITE,
                                    "sharepoint://sps2k7-01/testsite1/doclib21/abc.txt", //source 
                                    ref sourceAttributes,
                                    null, //target
                                    ref targetAttributes,
                                    999, //ip address
                                    user,
                                    true,
                                    CETYPE.CENoiseLevel_t.CE_NOISE_LEVEL_APPLICATION, //noise level
                                    out enforcement_obligation,
                                    out enforcement_result,
                                    5000 //timeout
                                );
            Console.WriteLine("CEEVALUATE_CheckPortal returns: {0}.", 
			    	result);	
            if (result == CETYPE.CEResult_t.CE_RESULT_SUCCESS)
            {
                 //Verify the result from CEEVALUATE_CheckResource
                Console.WriteLine("CEEVALUATE_CheckPortal result: {0} ({1}).",
                                   enforcement_result, 
                                   (enforcement_result==CETYPE.CEResponse_t.CEAllow)?"allow":"deny");

                for (int i = 0; i < enforcement_obligation.Length; i++)
                {
                    Console.WriteLine("CEEVALUATE_CheckPortal: {0} obligation key {1}.",
                    i, enforcement_obligation[i]);
                    i++;
                    Console.WriteLine("CEEVALUATE_CheckPortal: {0} obligation key {1}.",
                    i, enforcement_obligation[i]);
                }
               //End: Verify the result from CEEVALUATE_CheckResource
             }

             //Test CheckFile
             sourceAttributes[0] = CETYPE.CEAttrKey.CE_ATTR_MODIFIED_DATE;
             sourceAttributes[1] = "1234567";
             result = CESDKAPI.CEEVALUATE_CheckFile(connectHandle,
                                     CETYPE.CEAction_t.CE_ACTION_WRITE,
                                     "c:\\TEMP\\abc.txt", //source 
                                     ref sourceAttributes,
                                     null, //target
                                     ref targetAttributes,
                                     999, //ip address
                                     user,
                                     app,
                                     true,
                                     CETYPE.CENoiseLevel_t.CE_NOISE_LEVEL_APPLICATION, //noise level
                                     out enforcement_obligation,
                                     out enforcement_result,
                                     5000 //timeout
                                 );
             Console.WriteLine("CEEVALUATE_CheckFile returns: {0}.",
                     result);
             if (result == CETYPE.CEResult_t.CE_RESULT_SUCCESS)
             {
                 //Verify the result from CEEVALUATE_CheckResource
                 Console.WriteLine("CEEVALUATE_CheckFile result: {0} ({1}).",
                                    enforcement_result,
                                    (enforcement_result == CETYPE.CEResponse_t.CEAllow) ? "allow" : "deny");

                 for (int i = 0; i < enforcement_obligation.Length; i++)
                 {
                     Console.WriteLine("CEEVALUATE_CheckFile: {0} obligation key {1}.",
                     i, enforcement_obligation[i]);
                     i++;
                     Console.WriteLine("CEEVALUATE_CheckFile: {0} obligation key {1}.",
                     i, enforcement_obligation[i]);
                 }
                 //End: Verify the result from CEEVALUATE_CheckResource
             }

             //Test CheckMessageAttachment
             sourceAttributes[0] = CETYPE.CEAttrKey.CE_ATTR_MODIFIED_DATE;
             sourceAttributes[1] = "1234567";
             string[] recipients=new string[1];
             recipients[0] = "hzhou@nextlabs.com";
             string[] userAttributes = new string[2];
             string[] appAttributes = new string[2];
             userAttributes[0] = "user-attr-key";
             userAttributes[1] = "user-attr-value";
             appAttributes[0] = "app-attr-key";
             appAttributes[1] = "app-attr-value";
             result = CESDKAPI.CEEVALUATE_CheckMessageAttachment(connectHandle,
                                     CETYPE.CEAction_t.CE_ACTION_IM_FILE,
                                     "C:\\No_attachment.ice", //source 
                                     ref sourceAttributes,
                                     ref recipients,
                                     999, //ip address
                                     user,
                                     ref userAttributes,
                                     app,
                                     ref appAttributes,
                                     true,
                                     CETYPE.CENoiseLevel_t.CE_NOISE_LEVEL_APPLICATION, //noise level
                                     out enforcement_obligation,
                                     out enforcement_result,
                                     5000 //timeout
                                 );
             Console.WriteLine("CEEVALUATE_CheckMessageAttachment returns: {0}.",
                     result);
             if (result == CETYPE.CEResult_t.CE_RESULT_SUCCESS)
             {
                 //Verify the result from CEEVALUATE_CheckResource
                 Console.WriteLine("CEEVALUATE_CheckMessageAttachment result: {0} ({1}).",
                                    enforcement_result,
                                    (enforcement_result == CETYPE.CEResponse_t.CEAllow) ? "allow" : "deny");

                 for (int i = 0; i < enforcement_obligation.Length; i++)
                 {
                     Console.WriteLine("CEEVALUATE_CheckMessageAttachment: {0} obligation key {1}.",
                     i, enforcement_obligation[i]);
                     i++;
                     Console.WriteLine("CEEVALUATE_CheckMessageAttachment: {0} obligation key {1}.",
                     i, enforcement_obligation[i]);
                 }
                 //End: Verify the result from CEEVALUATE_CheckResource
             }

             //Test CELOGGING_LogObligationData
             string[] logAttributes = new string[1*2];
             //string[] logAttributes = null;
             logAttributes[0] = "logAttr-key-1";
             logAttributes[1] = "logAttr-value-1";
             result = CESDKAPI.CELOGGING_LogObligationData(connectHandle,
                                     "logIdentifier", //logIdentifier
                                     "assistantName", //assistantName
                                     ref logAttributes
                                 );
             Console.WriteLine("CELOGGING_LogObligationData returns: {0}.", result);

            //Test CECONN_Close
             result = CESDKAPI.CECONN_Close(connectHandle, 5000);
             Console.WriteLine("CECONN_Close returns: {0}.", result);
           }
        }
    }
}

