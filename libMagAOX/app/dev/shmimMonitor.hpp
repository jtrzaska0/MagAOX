/** \file shmimMonitor.hpp
  * \brief The MagAO-X generic shared memory monitor.
  *
  * \author Jared R. Males (jaredmales@gmail.com)
  *
  * \ingroup app_files
  */

#ifndef shmimMonitor_hpp
#define shmimMonitor_hpp


#include <ImageStruct.h>
#include <ImageStreamIO.h>

#include "../../common/paths.hpp"


namespace MagAOX
{
namespace app
{
namespace dev 
{

/** MagAO-X generic frame grabber
  *
  * 
  * The derived class `derivedT` must expose the following interface
   \code 
    
   \endcode  
  * Each of the above functions should return 0 on success, and -1 on an error. 
  * 
  * This class should be declared a friend in the derived class, like so:
   \code 
    friend class dev::shmimMonitor<derivedT>;
   \endcode
  *
  * Calls to this class's `setupConfig`, `loadConfig`, `appStartup`, `appLogic` and `appShutdown`
  * functions must be placed in the derived class's functions of the same name.
  *
  * \ingroup appdev
  */
template<class derivedT>
class shmimMonitor 
{
protected:

   /** \name Configurable Parameters
    * @{
    */
   std::string m_shmimName {""}; ///< The name of the shared memory image, is used in `/tmp/<shmimName>.im.shm`. Derived classes should set a default.
      
   int m_smThreadPrio {2}; ///< Priority of the shmimMonitor thread, should normally be > 00.
   
   ///@}
   
   int m_semaphoreNumber {0}; ///< The image structure semaphore index.
   
   uint32_t m_width {0}; ///< The width of the images in the stream
   uint32_t m_height {0}; ///< The height of the images in the stream
   
   uint8_t m_dataType{0}; ///< The ImageStreamIO type code.
   size_t m_typeSize {0}; ///< The size of the type, in bytes.  Result of sizeof.

   IMAGE m_imageStream; ///< The ImageStreamIO shared memory buffer.

public:

   /// Setup the configuration system
   /**
     * This should be called in `derivedT::setupConfig` as
     * \code
       shmimMonitor<derivedT>::setupConfig(config);
       \endcode
     * with appropriate error checking.
     */
   void setupConfig(mx::app::appConfigurator & config /**< [out] the derived classes configurator*/);

   /// load the configuration system results
   /**
     * This should be called in `derivedT::loadConfig` as
     * \code
       shmimMonitor<derivedT>::loadConfig(config);
       \endcode
     * with appropriate error checking.
     */
   void loadConfig(mx::app::appConfigurator & config /**< [in] the derived classes configurator*/);

   /// Startup function
   /** Starts the shmimMonitor thread
     * This should be called in `derivedT::appStartup` as
     * \code
       shmimMonitor<derivedT>::appStartup();
       \endcode
     * with appropriate error checking.
     * 
     * \returns 0 on success
     * \returns -1 on error, which is logged.
     */
   int appStartup();

   /// Checks the shmimMonitor thread
   /** This should be called in `derivedT::appLogic` as
     * \code
       shmimMonitor<derivedT>::appLogic();
       \endcode
     * with appropriate error checking.
     * 
     * \returns 0 on success
     * \returns -1 on error, which is logged.
     */
   int appLogic();

   /// Shuts down the shmimMonitor thread
   /** This should be called in `derivedT::appShutdown` as
     * \code
       shmimMonitor<derivedT>::appShutdown();
       \endcode
     * with appropriate error checking.
     * 
     * \returns 0 on success
     * \returns -1 on error, which is logged.
     */
   int appShutdown();
   
protected:
   
   /** \name SIGSEGV & SIGBUS signal handling
     * These signals occur as a result of a ImageStreamIO source server resetting (e.g. changing frame sizes).
     * When they occur a restart of the framegrabber and framewriter thread main loops is triggered.
     * 
     * @{
     */ 
   bool m_restart {false}; ///< Flag indicating tha the shared memory should be reinitialized.
   
   static shmimMonitor * m_selfMonitor; ///< Static pointer to this (set in constructor).  Used for getting out of the static SIGSEGV handler.

   ///Sets the handler for SIGSEGV and SIGBUS
   /** These are caused by ImageStreamIO server resets.
     */
   int setSigSegvHandler();
   
   ///The handler called when SIGSEGV or SIGBUS is received, which will be due to ImageStreamIO server resets.  Just a wrapper for handlerSigSegv.
   static void _handlerSigSegv( int signum,
                                siginfo_t *siginf,
                                void *ucont
                              );

   ///Handles SIGSEGV and SIGBUS.  Sets m_restart to true.
   void handlerSigSegv( int signum,
                        siginfo_t *siginf,
                        void *ucont
                      );
   ///@}
   
    /** \name shmimmonitor Thread
     * This thread actually monitors the shared memory buffer
     * @{
     */
   
   bool m_smThreadInit {true}; ///< Synchronizer for thread startup, to allow priority setting to finish.
   
   std::thread m_smThread; ///< A separate thread for the actual monitoring

   ///Thread starter, called by MagAOXApp::threadStart on thread construction.  Calls smThreadExec.
   static void smThreadStart( shmimMonitor * s /**< [in] a pointer to a shmimMonitor instance (normally this) */);

   /// Execute the monitoring thread
   void smThreadExec();
   
   ///@}
  
   
    
   
    /** \name INDI 
      *
      *@{
      */ 
protected:
   //declare our properties
   
   pcf::IndiProperty m_indiP_shmimName; ///< Property used to report the shmim buffer name
   
   pcf::IndiProperty m_indiP_frameSize; ///< Property used to report the current frame size

public:

   /// Update the INDI properties for this device controller
   /** You should call this once per main loop.
     * It is not called automatically.
     *
     * \returns 0 on success.
     * \returns -1 on error.
     */
   int updateINDI();

   ///@}
   
private:
   derivedT & derived()
   {
      return *static_cast<derivedT *>(this);
   }
};

//Set self pointer to null so app starts up uninitialized.
template<class derivedT>
shmimMonitor<derivedT> * shmimMonitor<derivedT>::m_selfMonitor = nullptr;



template<class derivedT>
void shmimMonitor<derivedT>::setupConfig(mx::app::appConfigurator & config)
{
   config.add("shmimMonitor.threadPrio", "", "shmimMonitor.threadPrio", argType::Required, "shmimMonitor", "threadPrio", false, "int", "The real-time priority of the shmimMonitor thread.");
   
   config.add("shmimMonitor.shmimName", "", "shmimMonitor.shmimName", argType::Required, "shmimMonitor", "shmimName", false, "string", "The name of the ImageStreamIO shared memory image. Will be used as /tmp/<shmimName>.im.shm.");
         
}

template<class derivedT>
void shmimMonitor<derivedT>::loadConfig(mx::app::appConfigurator & config)
{
   config(m_smThreadPrio, "shmimMonitor.threadPrio");
   m_shmimName = derived().configName();
   config(m_shmimName, "shmimMonitor.shmimName");
  
}
   

template<class derivedT>
int shmimMonitor<derivedT>::appStartup()
{
   //Register the shmimName INDI property
   m_indiP_shmimName = pcf::IndiProperty(pcf::IndiProperty::Text);
   m_indiP_shmimName.setDevice(derived().configName());
   m_indiP_shmimName.setName("shmimName");
   m_indiP_shmimName.setPerm(pcf::IndiProperty::ReadWrite); ///\todo why is this ReadWrite?
   m_indiP_shmimName.setState(pcf::IndiProperty::Idle);
   m_indiP_shmimName.add(pcf::IndiElement("name"));
   m_indiP_shmimName["name"] = m_shmimName;
   
   if( derived().registerIndiPropertyNew( m_indiP_shmimName, nullptr) < 0)
   {
      #ifndef SHMIMMONITOR_TEST_NOLOG
      derivedT::template log<software_error>({__FILE__,__LINE__});
      #endif
      return -1;
   }
   
   //Register the frameSize INDI property
   m_indiP_frameSize = pcf::IndiProperty(pcf::IndiProperty::Number);
   m_indiP_frameSize.setDevice(derived().configName());
   m_indiP_frameSize.setName("frameSize");
   m_indiP_frameSize.setPerm(pcf::IndiProperty::ReadWrite); ///\todo why is this ReadWrite?
   m_indiP_frameSize.setState(pcf::IndiProperty::Idle);
   m_indiP_frameSize.add(pcf::IndiElement("width"));
   m_indiP_frameSize["width"] = 0;
   m_indiP_frameSize.add(pcf::IndiElement("height"));
   m_indiP_frameSize["height"] = 0;
   
   if(setSigSegvHandler() < 0)
   {
      #ifndef SHMIMMONITOR_TEST_NOLOG
      derivedT::template log<software_error>({__FILE__,__LINE__});
      #endif
      return -1;
   }
      
   if( derived().registerIndiPropertyNew( m_indiP_frameSize, nullptr) < 0)
   {
      #ifndef SHMIMMONITOR_TEST_NOLOG
      derivedT::template log<software_error>({__FILE__,__LINE__});
      #endif
      return -1;
   }
   
   if(derived().threadStart( m_smThread, m_smThreadInit, m_smThreadPrio, "shmimMonitor", this, smThreadStart) < 0)
   {
      derivedT::template log<software_error>({__FILE__, __LINE__});
      return -1;
   }
   
   return 0;

}

template<class derivedT>
int shmimMonitor<derivedT>::appLogic()
{
   //do a join check to see if other threads have exited.
   if(pthread_tryjoin_np(m_smThread.native_handle(),0) == 0)
   {
      derivedT::template log<software_error>({__FILE__, __LINE__, "shmimMonitor thread has exited"});
      
      return -1;
   }
   
   return 0;

}


template<class derivedT>
int shmimMonitor<derivedT>::appShutdown()
{
   if(m_smThread.joinable())
   {
      try
      {
         m_smThread.join(); //this will throw if it was already joined
      }
      catch(...)
      {
      }
   }

   return 0;
}

template<class derivedT>
int shmimMonitor<derivedT>::setSigSegvHandler()
{
   struct sigaction act;
   sigset_t set;

   act.sa_sigaction = &shmimMonitor<derivedT>::_handlerSigSegv;
   act.sa_flags = SA_SIGINFO;
   sigemptyset(&set);
   act.sa_mask = set;

   errno = 0;
   if( sigaction(SIGSEGV, &act, 0) < 0 )
   {
      std::string logss = "Setting handler for SIGSEGV failed. Errno says: ";
      logss += strerror(errno);

      derivedT::template log<software_error>({__FILE__, __LINE__, errno, 0, logss});

      return -1;
   }

   errno = 0;
   if( sigaction(SIGBUS, &act, 0) < 0 )
   {
      std::string logss = "Setting handler for SIGBUS failed. Errno says: ";
      logss += strerror(errno);

      derivedT::template log<software_error>({__FILE__, __LINE__, errno, 0,logss});

      return -1;
   }

   derivedT::template log<text_log>("Installed SIGSEGV/SIGBUS signal handler.", logPrio::LOG_DEBUG);

   return 0;
}

template<class derivedT>
void shmimMonitor<derivedT>::_handlerSigSegv( int signum,
                                              siginfo_t *siginf,
                                              void *ucont
                                            )
{
   m_selfMonitor->handlerSigSegv(signum, siginf, ucont);
}

template<class derivedT>
void shmimMonitor<derivedT>::handlerSigSegv( int signum,
                                             siginfo_t *siginf,
                                             void *ucont
                                           )
{
   static_cast<void>(signum);
   static_cast<void>(siginf);
   static_cast<void>(ucont);
   
   m_restart = true;

   return;
}

template<class derivedT>
void shmimMonitor<derivedT>::smThreadStart( shmimMonitor * s)
{
   s->smThreadExec();
}


template<class derivedT>
void shmimMonitor<derivedT>::smThreadExec()
{
   //Wait for the thread starter to finish initializing this thread.
   while( m_smThreadInit == true && derived().shutdown() == 0)
   {
      sleep(1);
   }
   
   bool opened = false;
   
   
      
   while(derived().shutdown() == 0)
   {
      /* Initialize ImageStreamIO
       */
      opened = false;
      m_restart = false; //Set this up front, since we're about to restart.
      
      while(!opened && !derived().m_shutdown && !m_restart)
      {
         if( ImageStreamIO_openIm(&m_imageStream, m_shmimName.c_str()) == 0)
         {
            if(m_imageStream.md[0].sem <= m_semaphoreNumber) ///<\todo this isn't right--> isn't there a define in cacao to use? 
            {
               ImageStreamIO_closeIm(&m_imageStream);
               mx::sleep(1); //We just need to wait for the server process to finish startup.
            }
            else
            {
               opened = true;
            }
         }
         else
         {
            mx::sleep(1); //be patient
         }
      }
      
      if(derived().m_shutdown || !opened) return;
    
      m_semaphoreNumber = ImageStreamIO_getsemwaitindex(&m_imageStream, m_semaphoreNumber); //ask for semaphore we had before
      if(m_semaphoreNumber == -1)
      {
         derivedT::template log<software_critical>({__FILE__,__LINE__, "could not get semaphore index"});
         return;
      }
      
      derivedT::template log<software_info>({__FILE__,__LINE__, "got semaphore index " + std::to_string(m_semaphoreNumber) });
      
      sem_t * sem = m_imageStream.semptr[m_semaphoreNumber]; ///< The semaphore to monitor for new image data
      
      m_dataType = m_imageStream.md[0].datatype;
      m_typeSize = ImageStreamIO_typesize(m_dataType);
      m_width = m_imageStream.md[0].size[0];
      m_height = m_imageStream.md[0].size[1];
      size_t length = m_imageStream.md[0].size[2];

      
      if( derived().allocate() < 0)
      {
         derivedT::template log<software_critical>({__FILE__,__LINE__});
         break;
      }
      
      uint8_t atype;
      size_t snx, sny, snz;
      uint64_t curr_image; //The current cnt1 index
      
      //This is the main image grabbing loop.
      while( derived().shutdown() == 0 && !m_restart)
      {
         
         timespec ts;
         
         if(clock_gettime(CLOCK_REALTIME, &ts) < 0)
         {
            derivedT::template log<software_critical>({__FILE__,__LINE__,errno,0,"clock_gettime"}); 
            return;
         }
         
         ts.tv_sec += 1;
         
         if(sem_timedwait(sem, &ts) == 0)
         {
            if(m_imageStream.md[0].size[2] > 0) ///\todo change to naxis?
            {
               curr_image = m_imageStream.md[0].cnt1;
            }
            else curr_image = 0;

            atype = m_imageStream.md[0].datatype;
            snx = m_imageStream.md[0].size[0];
            sny = m_imageStream.md[0].size[1];
            snz = m_imageStream.md[0].size[2];
         
            if( atype!= m_dataType || snx != m_width || sny != m_height || snz != length )
            {
               break; //exit the nearest while loop and get the new image setup.
            }
         
            if(derived().shutdown() != 0 || m_restart) break; //Check for exit signals
         
            char * curr_src = (char *)  m_imageStream.array.raw + curr_image*m_width*m_height*m_typeSize;
            
            if( derived().processImage(curr_src) < 0)
            {
               derivedT::template log<software_error>({__FILE__,__LINE__});
            }
         }
         else
         {
            if(m_imageStream.md[0].sem <= 0) break; //Indicates that the server has cleaned up.
            
            //Check for why we timed out
            if(errno == EINTR) break; //This indicates signal interrupted us, time to restart or shutdown, loop will exit normally if flags set.
            
            //ETIMEDOUT just means we should wait more.
            //Otherwise, report an error.
            if(errno != ETIMEDOUT)
            {
               derivedT::template log<software_error>({__FILE__, __LINE__,errno, "sem_timedwait"});
               break;
            }
         }
      }
       
      //*******
      // call derived().cleanup()
      //*******
      
      if(opened) 
      {
         ImageStreamIO_closeIm(&m_imageStream);
         opened = false;
      }
      
   } //outer loop, will exit if m_shutdown==true
      
      
   //*******
   // call derived().cleanup()
   //*******   
   
   if(opened) 
   {
      ImageStreamIO_closeIm(&m_imageStream);
      opened = false;
   }
   
}





template<class derivedT>
int shmimMonitor<derivedT>::updateINDI()
{
   if( !derived().m_indiDriver ) return 0;
   
   indi::updateIfChanged(m_indiP_shmimName, "name", m_shmimName, derived().m_indiDriver);                     
   indi::updateIfChanged(m_indiP_frameSize, "width", m_width, derived().m_indiDriver);
   indi::updateIfChanged(m_indiP_frameSize, "height", m_height, derived().m_indiDriver);
   
   
   return 0;
}


} //namespace dev
} //namespace app
} //namespace MagAOX
#endif
