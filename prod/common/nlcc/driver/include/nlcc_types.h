
#ifndef __DSCORE_TYPES_H__
#define __DSCORE_TYPES_H__

/** DriverContext
 *
 *  \brief Context for the DSCORE driver.
 */
typedef struct
{
  /* This index is used to generate unique ID's for transactions with the PDP.
   * The value is used to assign to request and then to match with responses
   * from the PDP.  This value should *only* be access using an interlocked
   * method.
   */
  volatile LONGLONG tx_id;

  /* PDP queus */
  WDFQUEUE ioq_pdp_default;   /* Default queue */
  WDFQUEUE ioq_pdp_wait;      /* PDP queue - wait for request */
  WDFQUEUE ioq_pep_wait;      /* PEP queue - wait for response */

  WDFQUEUE ioq_service_wait;  /* PDP is processing PEP request  */

  /* PEP queues */
  WDFQUEUE ioq_pep_default;  /* Default queue */

  WDFIOTARGET iot;
  WDFWAITLOCK iot_lock;
  volatile LONG pep_count;
  volatile LONG pdp_count;

  /* I/O queue lock.  This lock protects the fetch-forward pattern and is
   * required to prevent a race condition which leads to both the PEP and
   * PDP waiting for each other to complete a request for policy query.
   */
  WDFSPINLOCK qlock;

  //This lock protects ioq_service_wait queue
  WDFSPINLOCK qServiceWaitlock;
} DriverContext;

/** RequestContext
 *
 *  \brief Context for a WDFREQUEST.  This context is create on requests which are pending
 *         a response from the PDP.  The ID is used to match the response to the request.
 */
typedef struct
{
  volatile LONGLONG tx_id;
} RequestContext;

#endif /* __DSCORE_TYPES_H__ */
