
/***************************************************************************
 *
 * High resolution timer
 *
 **************************************************************************/

#ifndef __HTIMER_HPP__
#define __HTIMER_HPP__

namespace nextlabs
{
  class high_resolution_timer
  {

    private:
      double frequency;
      LARGE_INTEGER start_time;
      LARGE_INTEGER end_time;

    public:
    
      high_resolution_timer()
      {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	frequency = (double)freq.QuadPart/(double)1000;
	QueryPerformanceCounter(&start_time);
      }/* high_resolution_timer */

      void start(void)
      {
	QueryPerformanceCounter(&start_time);
      }/* start */

      void stop(void)
      {
	QueryPerformanceCounter(&end_time);
      }/* stop */

      /** diff
       *
       *  \brief Return the number of milliseconds since the timer was started.
       */
      double diff(void)
      {
	stop();
	return (double)(end_time.QuadPart - start_time.QuadPart)/(double)frequency;
      }/* diff */

  };
}/* namespace nextlabs */

#endif /* __HTIMER_HPP__ */
