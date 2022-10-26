/**********************************************************************************************
 *
 * NLCC Policy Controller Service
 *
 *********************************************************************************************/

package com.bluejungle.NLCC;

import java.util.*;
import java.io.File;
import java.io.FileNotFoundException;
import java.lang.Error;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
    
import com.bluejungle.destiny.agent.pdpapi.*;
import com.bluejungle.pf.domain.destiny.serviceprovider.IExternalServiceProvider;
import com.bluejungle.pf.domain.destiny.serviceprovider.IExternalServiceProviderResponse;
import com.bluejungle.pf.domain.destiny.serviceprovider.ServiceProviderException;

/** NLCCServiceCallback
 *
 *  \brief This class implements the callback for the PDP API.  It will deliver a response
 *         to the native code dispatcher when called.
 */
class NLCCServiceCallback implements IPDPSDKCallback
{
    private final Log LOG = LogFactory.getLog(NLCCService.class);
    private static final NLCCServiceDispatcher dispatcher = new NLCCServiceDispatcher();
    private long tx_id = 0;
    private long start_time = 0;

    /* Construct the callback with a transaction ID */
    NLCCServiceCallback( long in_tx_id )
    {
        tx_id = in_tx_id;
        start_time = System.nanoTime();
    }/* NLCCServiceCallback */

    public void callback( IPDPEnforcement enf )
    {
        boolean is_allow = true;

        if( enf.getResult().equals("deny") == true )
        {
            is_allow = false;
        }

        if( LOG.isDebugEnabled() )
        {
            long elapsed_time = System.nanoTime() - start_time;
            LOG.debug("NLCCServiceCallback: is_allow = " + is_allow + ", TX ID = " + tx_id + ", time = " + (elapsed_time / 1000000) + "ms");
        }

        Object[] obs = enf.getObligations();
        LOG.debug("NLCCServiceCallback: send response");
        dispatcher.SendResult(tx_id,is_allow,obs);
        LOG.debug("NLCCServiceCallback: complete");
    }/* callback */

}/* NLCCServiceCallback */

/** NLCCServiceDispatcher
 *
 *  \brief JNI / Native interface to user-mode adapter
 *
 ***************************************************************************************/
class NLCCServiceDispatcher
{

    /** Open
     *
     *  \brief Open the device.
     *
     *  \return true on a success open, otherwise false.
     */
    public native boolean Open();

    /** ReadRequest
     *
     *  \brief Read a request from the device.
     *
     *  \return Attribute set.  The first attribute contains the transaction ID.  The second
     *          attribute is reserved.  The remaining attributes are PEP set.
     */
    public native Object[] ReadRequest();

    /** SendResult
     *
     *  \brief Send a PDP decision to the NLCC dispatcher.
     *
     *  \param in_tx_id (in)    Transaction ID.  The transaction ID is retrieved from ReadRequest.
     *  \param allow (in)       Decision is allow?
     *  \param obligations (in) Obligations to return to the PEP.
     *
     *  \return true if the result was delivered, otherwise false.
     *
     *  \sa ReadRequest
     */
    public native boolean SendResult( long in_tx_id ,
                                      boolean allow ,
                                      Object[] obligations );

}/* NLCCServiceDispatcher */

/** NLCCServiceThread
 *
 *  \brief Implement the NLCC Policy Controller service
 */
class NLCCServiceThread extends Thread
{
    private final Log LOG = LogFactory.getLog(NLCCServiceThread.class);
    private static final NLCCServiceDispatcher dispatcher = new NLCCServiceDispatcher();

    private static final String ACTION_MONITOR_APP = "ce::monitor_application";
    private static final String SOURCE_ATTR_PREFIX = "source_attr_";
    private static final String TARGET_ATTR_PREFIX = "target_attr_";

    private static final IPDPHost LOCALHOST = new PDPHost(0x0100007f);  /* 127.0.0.1 */

    public void run()
    {
        try
        {
            LOG.debug("NLCCService: run: run_wrapper");
            run_wrapper();
        }
        catch( Throwable e )
        {
            LOG.error("NLCCService: run: run_wrapper caught an exception - " + e);
        }
    }

    public void run_wrapper()
    {
        for ( ; ; )
        {
            Map<String,String> other_attrs = new HashMap<String, String>();
            Map<String,String> source_attrs = new HashMap<String, String>();
            Map<String,String> target_attrs = new HashMap<String, String>();
            boolean isInBlackList = false;

            Object attrs[];
            LOG.debug("NLCCService: run: read request");
            attrs = dispatcher.ReadRequest();
            LOG.debug("NLCCService: run: read request - complete");

            if( attrs == null )
            {
                LOG.error("NLCCService: run: ReadRequest failed");
                throw new Error("NLCCService: run: ReadRequest failed");
            }

            /* The transaction ID must be retained for the query.  The NLCC driver uses this ID to match
             * request to response in the pending query queue.
             */
            String tx_id_string = (String)attrs[0];
            long tx_id = Long.parseLong(tx_id_string);

            StringBuilder sb = new StringBuilder("NLCCService: Request\n");
            sb.append("  tx_id = " + tx_id + "\n");
            
            //add blacklist capabilities
            File inFile = new File("./config", "blacklist.cfg");
            if (!inFile.exists() || !inFile.canRead()) {
            	LOG.info("blacklist configuration information is not available.");
            }
            
            ArrayList<String> blackList = new ArrayList<String>();
            
            try {
            	Scanner s = new Scanner(inFile);
                //add all blacklist regular expressions into arrayList
                while (s.hasNext()){
                	blackList.add(s.next().toLowerCase());
                }
                s.close();
        	} catch (FileNotFoundException ex) {
        		ex.printStackTrace();             
        	}
        	

            for( int i = 2 ; i < attrs.length ; i += 2 )
            {
                String key, value;

                key = (String)attrs[i];
                value = (String)attrs[i+1];
                
                //check if request matches one of the regular expression inside blacklist	
                if(isInBlackList == false)
                {
	                for(int j = 0; j<blackList.size(); j++)
	                {
	                	String currRegExp = blackList.get(j); 
		                if(value.toLowerCase().matches(currRegExp))
		                {
		                	LOG.debug("A value inside the request matched a regular expression inside blacklist. " + value + "matched regular expression: " + currRegExp);
		                	//return allow
		                	String obligations[] = {};
		                	NLCCServiceCallback cb = new NLCCServiceCallback(tx_id);
		                	IPDPEnforcement enforcement = new PDPEnforcement("allow", obligations);
		                	cb.callback(enforcement);
		                	isInBlackList = true;
		                	break; 
		                }
	                }
                }
                if( key == null || value == null ||
                    key.length() == 0 || value.length() == 0 )
                {
                    LOG.error("attribute key or value is null or empty at index" + i);
                    continue;
                }

                sb.append("  " + key + " = " + value + "\n");

                if( key.startsWith(SOURCE_ATTR_PREFIX) == true )
                {
                    source_attrs.put(key,value);
                }
                else if( key.startsWith(TARGET_ATTR_PREFIX) == true )
                {
                    target_attrs.put(key,value);
                }
                else
                {
                    other_attrs.put(key,value);
                }
            }
            if(isInBlackList == false)
            {
	            LOG.debug(sb.toString());
	            LOG.debug("NLCCService: perform query");
	            try
	            {
	            	PerformQuery(tx_id,tx_id_string,other_attrs,source_attrs,target_attrs);
	            }
	            catch( IllegalArgumentException e )
	            {
	                LOG.error("run: PerformQuery: exception: " + e);
	            }
	            catch( PDPTimeout e )
	            {
	                LOG.error("run: PerformQuery: exception: " + e);
	            }
	            catch( PDPException e )
	            {
	                LOG.error("run: PerformQuery: exception: " + e);
	            }
	            LOG.debug("NLCCService: response complete - fire and forget");
            }
        }
    }/* run */

    public void PerformQuery( long tx_id ,
                              String tx_id_string , 
                              Map<String,String> attrs ,
                              Map<String,String> source_attrs ,
                              Map<String,String> target_attrs )
        throws PDPTimeout, PDPException, IllegalArgumentException
    {
        String attr_action      = attrs.get("action");
        String attr_perf_obs    = attrs.get("perf_obs");
        String attr_event_level = attrs.get("event_level");
        String attr_pid         = attrs.get("pid");
        String attr_source      = attrs.get("source_name");
        String attr_source_type = attrs.get("source_type");
        String attr_user_id     = attrs.get("user_id");
        String attr_target      = attrs.get("target_name");
        String attr_target_type = attrs.get("target_type");
        String attr_application = attrs.get("application");
        String attr_host = attrs.get("ip");
        boolean is_allow = true;
        IPDPHost host = LOCALHOST; /* default to localhost */
        IPDPResource[] resources = null;

        int resource_count = 1;
        if( attr_target != null )
        {
            resource_count = 2;
        }
        resources = new IPDPResource[resource_count];

        if( LOG.isDebugEnabled() )
        {
            LOG.debug("PerformQuery: <" + attr_source + "," + attr_source_type + ">" );
        }

        /* Monitor application uses an empty resource */
        if( attr_action.equals(ACTION_MONITOR_APP) == true )
        { 
            resources[0] = PDPResource.NONE;
        }
        else
        {
            resources[0] = new PDPResource("from",attr_source,attr_source_type);
        }

        if( attr_target != null )
        {
            resources[1] = new PDPResource("to",attr_target,attr_target_type);
        }

        /* Process source resource attributes */
        if( resources[0] != PDPResource.NONE )
        {
            for (Map.Entry<String, String> pair : source_attrs.entrySet()) {
                String key = pair.getKey();
                String value = pair.getValue();
                String new_key = key.substring(SOURCE_ATTR_PREFIX.length());
                resources[0].setAttribute(new_key,value);
            }

            /* Process target resource attributes */
            for (Map.Entry<String, String> pair : target_attrs.entrySet())
            {
                String key = pair.getKey();
                String value = pair.getValue();
                String new_key = key.substring(TARGET_ATTR_PREFIX.length());
                resources[1].setAttribute(new_key,value);
            }
        }

        IPDPUser user = new PDPUser(attr_user_id);

        /* @@ TODO - IPCJNI (oswrapper) requires a new API which is not quite working yet.  Use
         *           a PID of 0 to work around this.
         */
        long pid = 0; //Long.parseLong(attr_pid);
      
        IPDPApplication app = PDPApplication.NONE;
        /* Application is an optional field */
        if( attr_application != null )
        {
            app = new PDPApplication(attr_application,pid);
        }

        /* Add the transaction ID to the agentlog */
        IPDPNamedAttributes[] info = null;
        if( LOG.isDebugEnabled() )
        {
            info = new PDPNamedAttributes[1];
            info[0] = new PDPNamedAttributes("info");
            info[0].setAttribute("tx id",tx_id_string);
        }

        /* Obligations should be performed?  By default, yes. */
        boolean perform_obligations = true;
        if( attr_perf_obs != null && attr_perf_obs.equals("false") )
        {
            perform_obligations = false;
        }

        /* Event level.  Default of 3 if the level is not defined. */
        int event_level = 3;
        if( attr_event_level != null )
        {
            event_level = Integer.parseInt(attr_pid);
        }

		if(attr_host != null)
		{
			int ip = Integer.parseInt(attr_host);
			host = new PDPHost(ip);
		}
		
        NLCCServiceCallback cb = new NLCCServiceCallback(tx_id);
        PDPSDK.PDPQueryDecisionEngine(attr_action,resources,user,app,host,
                                      perform_obligations,info,event_level,0,cb);
    }/* PerformQuery */

}/* NLCCServiceThread */

/** NLCCService
 *
 *  \brief Implement the NLCC Policy Controller service
 */
public class NLCCService implements IExternalServiceProvider
{
    private final Log LOG = LogFactory.getLog(NLCCService.class);
    private final NLCCServiceDispatcher dispatcher = new NLCCServiceDispatcher();
    NLCCServiceThread[] st;

    private boolean LoadNativeLibrary( String in_path )
    {
        boolean status = true;

        String cwd = System.getProperty("user.dir");
        String path = cwd + "\\" + in_path;
        try
        {
            System.load(path);
        }
        catch( UnsatisfiedLinkError e )
        {
            LOG.error("NLCCService: init: caught an exception for " + path + " - " + e);
            status = false;
        }
        return status;
    }

    public void init() throws ServiceProviderException
    {
        LOG.info("NLCCService: init: begin");

        if( LoadNativeLibrary("jservice\\jar\\nlcc\\nlcc_dispatcher.dll") == false )
        {
            if( LoadNativeLibrary("jservice\\jar\\nlcc\\nlcc_dispatcher32.dll") == false )
            {
                throw new ServiceProviderException("Can't find nlcc_dispatcher");
            }
        }

        LOG.info("NLCCService: init: open");
        boolean result = false;
        try
        {
            result = dispatcher.Open();
        }
        catch ( Throwable e )
        {
            result = false;
            LOG.error("NLCCService: init: NLCCServiceDispatcher::Open exception" + e);
            throw new ServiceProviderException("Failure opening dispatcher",e);
        }

        if( result == false )
        {
            LOG.error("NLCCService: init: Cannot open dispatcher");
            throw new ServiceProviderException("Failure opening dispatcher");
        }

        LOG.info("NLCCService: init: starting thread");

        st = new NLCCServiceThread[6];
        for( int i = 0 ; i < 6 ; i++ )
        {
            st[i] = new NLCCServiceThread();
            st[i].start();
        }
        LOG.info("NLCCService: init: complete");
    }/* init */

    public IExternalServiceProviderResponse invoke(Object[] objs)
    {
        return null;
    }/* invoke */

}/* NLCCService */
