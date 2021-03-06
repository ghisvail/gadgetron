#ifndef GADGETSTREAMINTERFACE_H
#define GADGETSTREAMINTERFACE_H

#include "ace/Stream.h"
#include "ace/DLL.h"
#include "ace/DLL_Manager.h"

#include "gadgetron_paths.h"
#include "Gadget.h"

typedef ACE_Module<ACE_MT_SYNCH> GadgetModule;

/**
   Abstract class for structure containing stream of Gadgets.

 */
namespace Gadgetron {

  class GadgetStreamInterface
  {
  public:
    GadgetStreamInterface()
      : stream_configured_(false)
    {  
      gadgetron_home_ = get_gadgetron_home();
    } 

    virtual int output_ready(ACE_Message_Block* mb) = 0;

    virtual Gadget* find_gadget(std::string gadget_name)
    {
      GadgetModule* gm = stream_.find(gadget_name.c_str());
      
      if (gm) {
	Gadget* g = dynamic_cast<Gadget*>(gm->writer());
	return g;
      } else {
	GDEBUG("Gadget with name %s not found! Returning null pointer\n", gadget_name.c_str());
      }
      
      return 0;
    }

    void set_global_gadget_parameters(const std::map<std::string, std::string>& globalGadgetPara)
    {
      global_gadget_parameters_ = globalGadgetPara;
    }

  protected:
    ACE_Stream<ACE_MT_SYNCH> stream_;
    bool stream_configured_;  
    std::vector<ACE_DLL_Handle*> dll_handles_;
    std::map<std::string, std::string> global_gadget_parameters_;
    std::string gadgetron_home_;
    
    virtual GadgetModule * create_gadget_module(const char* DLL, const char* gadget, const char* gadget_module_name)
    {

      Gadget* g = load_dll_component<Gadget>(DLL,gadget);
      
      if (!g) {
	GERROR("Failed to load gadget using factory\n");
	return 0;
      }
      
      g->set_controller(this);
      
      GadgetModule *module = 0;
      ACE_NEW_RETURN (module,
		      GadgetModule (gadget_module_name, g),
		      0);
      
      return module;
    }

    template <class T>  T* load_dll_component(const char* DLL, const char* component_name)
    {
      ACE_DLL_Manager* dllmgr = ACE_DLL_Manager::instance();
      
      ACE_DLL_Handle* dll = 0;
      ACE_SHLIB_HANDLE dll_handle = 0;
      
      ACE_TCHAR dllname[1024];
#if defined(WIN32) && defined(_DEBUG)
      ACE_OS::sprintf(dllname, "%s%sd",ACE_DLL_PREFIX, DLL);
#else
      ACE_OS::sprintf(dllname, "%s%s",ACE_DLL_PREFIX, DLL);
#endif

      ACE_TCHAR factoryname[1024];
      ACE_OS::sprintf(factoryname, "make_%s", component_name);
      
      dll = dllmgr->open_dll (dllname, ACE_DEFAULT_SHLIB_MODE, dll_handle );
      
      if (!dll) {
	GERROR("Failed to load DLL, Possible reasons: \n");
	GERROR("   * Name of DLL is wrong in XML file \n");
	GERROR("   * Path of DLL is not in your DLL search path (LD_LIBRARY_PATH on Unix)\n");
	GERROR("   * Path of other DLLs that this DLL depends on is not in the search path\n");
	return 0;
      } else {
	dll_handles_.push_back(dll);
      }

      //Function pointer
      typedef T* (*ComponentCreator) (void);
      
      void *void_ptr = dll->symbol (factoryname);
      ptrdiff_t tmp = reinterpret_cast<ptrdiff_t> (void_ptr);
      ComponentCreator cc = reinterpret_cast<ComponentCreator> (tmp);
      
      if (cc == 0) {
	GERROR("Failed to load factory (%s) from DLL (%s)\n", dllname, factoryname);
	return 0;
      }
      
      T* c = cc();
      
      if (!c) {
	GERROR("Failed to create component using factory\n");
	return 0;
      }
      
      return c;
    }
  };
}

#endif //GADGETSTREAMINTERFACE_H
