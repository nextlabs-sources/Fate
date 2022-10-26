/*
 * Created on May 29, 2012
 *
 * All sources, binaries and HTML pages (C) copyright 2012 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

import java.util.ArrayList;
import java.util.List;

import com.nextlabs.destiny.sdk.CEApplication;
import com.nextlabs.destiny.sdk.CEAttributes;
import com.nextlabs.destiny.sdk.CEEnforcement;
import com.nextlabs.destiny.sdk.CENamedAttributes;
import com.nextlabs.destiny.sdk.CEResource;
import com.nextlabs.destiny.sdk.CERequest;
import com.nextlabs.destiny.sdk.CESdk;
import com.nextlabs.destiny.sdk.CESdkException;
import com.nextlabs.destiny.sdk.CEUser;
import com.nextlabs.destiny.sdk.ICESdk;

public class SDKTest {
    public static final void main(String[] args) {
        CEApplication application = new CEApplication("sdktest.exe",
                                                      "c:/test/sdktest.exe");
        CEUser georgeWashington = new CEUser("george.washington@test.bluejungle.com",
                                             "S-1-5-21-830805687-550985140-3285839444-1197");
        CEUser groverCleveland = new CEUser("grover.cleveland@test.bluejungle.com",
                                            "S-1-5-21-830805687-550985140-3285839444-1165");
        CEResource source = new CEResource("c:/foo.java", "fso");
        CEResource dest = new CEResource("c:/foo.txt", "fso");
        String[] recipients = { "abraham.lincoln@nextlabs.com",
                                "john.tyler@nextlabs.com" };

        try {
            // This loop is added to test the timeout, while making continuous
            // requests.
            int localHost = CESdk.ipAddressToInteger("127.0.0.1");
            System.out.println("localHost IP="
                               + Integer.toHexString(localHost));

            // Below line initializes the SDK with default port - 1099
            // long h = sdk.Initialize(application, user, null, 10000);

            // Below line initializes the SDK with specific port - 11099
            CESdk sdk = new CESdk(null);
            CEEnforcement enforcementResult = sdk.checkResources("RENAME",
                                                                 source, null,
                                                                 dest, null,
                                                                 georgeWashington, null,
                                                                 application, null,
                                                                 null,
                                                                 recipients,
                                                                 localHost,
                                                                 true, // perform obligations
                                                                 ICESdk.CE_NOISE_LEVEL_USER_ACTION,
                                                                 100);
            System.out.println("Query result: "
                               + enforcementResult.getResponseAsString());
            System.out.println("Obligations:");
            for (String s : enforcementResult.getObligations().toArray()) {
                System.out.println(s);
            }

            String logIdentifier = "20";
            String obligationName = "MyObligationTest";

            String[] attributesArray = { "options", "aOptions", "desc",
                                         "aDescription", "actions", "aUserActions" };
            CEAttributes attributes = new CEAttributes(attributesArray);
            System.out.println("Calling LogObligationData");
            sdk.logObligationData(logIdentifier, obligationName, attributes);


            // Multi-query
            List<CERequest> requests = new ArrayList<CERequest>();

            CENamedAttributes[] additionalAttributes = new CENamedAttributes[1];

            additionalAttributes[0] = new CENamedAttributes("environment");
            additionalAttributes[0].add("skycolor", "blue");
            additionalAttributes[0].add("who's the man?", "you the man");

            for (int i = 0; i < 50; i++) {
                CEUser user = (i % 2 == 0) ? georgeWashington : groverCleveland;

                String action;

                switch (i % 7) {
                    case 0:
                        action = "OPEN";
                        break;
                    case 3:
                    case 4:
                        action = "DELETE";
                        break;
                    default:
                        action = "EMAIL";
                        break;
                }

                requests.add(new CERequest(action,
                                           new CEResource("c:/file" + i + ".txt", "fso"), null,
                                           null, null,
                                           user, null,
                                           application, null,
                                           null,
                                           (i % 11 == 0) ? additionalAttributes : null,
                                           true, ICESdk.CE_NOISE_LEVEL_USER_ACTION));
                
            }


            List<CEEnforcement> results = sdk.checkResources(requests,
                                                             "policy no_email for * on EMAIL by * do deny", false,
                                                             localHost,
                                                             10000);

            int i = 0;
            for (CEEnforcement result : results) {
                System.out.println("Multi query result: " + (i++) + ":" + result.getResponseAsString());
            }
        } catch (CESdkException e) {
            System.out.println("Oh the humanity.\n" + e);
            e.printStackTrace();
            if (e.getCause() instanceof java.rmi.RemoteException) {
                // The remote server is not useable. It may be re-starting or
                // offline.
                // Need to call sdk.Initialize to get a new handle
                // until we get one that works.
            }
        }
    }
}
