package com.bluejungle.EDP;
import java.io.*;
import java.lang.*;

// for interface
import com.bluejungle.pf.domain.destiny.serviceprovider.ExternalServiceProviderResponse;
import com.bluejungle.pf.domain.destiny.serviceprovider.IExternalServiceProvider;
import com.bluejungle.pf.domain.destiny.serviceprovider.IExternalServiceProviderResponse;
import com.bluejungle.pf.domain.destiny.serviceprovider.ServiceProviderException;

// Apache jar for logging
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class SystemEncryptionService implements IExternalServiceProvider{
  private final Log log = LogFactory.getLog(SystemEncryptionService.class);

  private boolean LoadNativeLibrary(String in_path)
  {
      boolean status = true;

      String cwd = System.getProperty("user.dir");
      String path = cwd + File.separatorChar + in_path;
      try
      {
          System.load(path);
      }
      catch (UnsatisfiedLinkError e)
      {
          log.error("SystemEncryptionService: init: can't find " + path + " - " + e);
          status = false;
      }
      catch (SecurityException e)
      {
          log.error("SystemEncryptionService: init: caught an exception for " + path + " - " + e);
          status = false;
      }
      return status;
  }

  public void init() throws ServiceProviderException
  {
      log.info("SystemEncryptionService: init");

      if (LoadNativeLibrary("jservice/SystemEncryption/pcs_server.dll") == false)
      {
          log.warn("Unable to load pcs_server.dll.  Trying pcs_server32.dll");
          if (LoadNativeLibrary("jservice/SystemEncryption/pcs_server32.dll") == false)
          {
	      throw new ServiceProviderException("Cannot load pcs_server[32].dll");
          }
      }

      Initialize();
      log.info("SystemEncryptionService: init done");
  }

  public IExternalServiceProviderResponse invoke(Object[] inobjects){
      log.debug("SystemEncryptionService.invoke()");
      log.debug("SystemEncryptionService.invoke: length "+inobjects.length);
      ExternalServiceProviderResponse response = new ExternalServiceProviderResponse();
      String Marshalvals = "empty";

      if (inobjects.length > 0)
      {
	  Marshalvals = (String) inobjects[0];
          log.debug("SystemEncryptionService::IExternalServiceProviderResponse: payload: "+Marshalvals);
      }

      response.setData(inobjects);
      response.setFormatString("s");

      log.debug("SystemEncryptionService::IExternalServiceProviderResponse: dispatch to native");

      String output = Dispatch(Marshalvals);

      log.debug("SystemEncryptionService::IExternalServiceProviderResponse: response: " + output);

      inobjects[0] = (Object)output;

      log.debug("SystemEncryptionService::IExternalServiceProviderResponse: complete");

      return response;
  }

  public native void Initialize();
  public native String Dispatch( String input );

}
