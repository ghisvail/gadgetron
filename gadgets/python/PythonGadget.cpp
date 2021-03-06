#include "PythonGadget.h"

namespace Gadgetron{
  int PythonGadget::process(ACE_Message_Block* mb)
  {
    GadgetContainerMessage<ISMRMRD::AcquisitionHeader>* hma = AsContainerMessage<ISMRMRD::AcquisitionHeader>(mb);
    if (hma) {
      GadgetContainerMessage< hoNDArray< std::complex<float> > >* dmb = AsContainerMessage< hoNDArray< std::complex<float> > >(hma->cont());
      if (!dmb) {
	GERROR("Error converting data array from message block for Acquisition\n");
	hma->release();
	return GADGET_FAIL;;
      }
      GadgetContainerMessage< ISMRMRD::MetaContainer>* mmb = AsContainerMessage< ISMRMRD::MetaContainer >(dmb->cont());
      this->process(hma,dmb,mmb);
    } else {
      GadgetContainerMessage<ISMRMRD::ImageHeader>* hmi = AsContainerMessage<ISMRMRD::ImageHeader>(mb);
      if (!hmi) {
	if (pass_on_undesired_data.value()) {
	  return this->next()->putq(mb);
	} else {
	  GERROR("This is neither an acquisition or an image. Something is wrong here");
	  mb->release();
	  return GADGET_FAIL;
	}
      }
      
      ISMRMRD::ImageHeader* h = hmi->getObjectPtr();
      GadgetContainerMessage< ISMRMRD::MetaContainer>* mmb = 0;

      if (hmi->cont()) {
	mmb = AsContainerMessage< ISMRMRD::MetaContainer >(hmi->cont()->cont());
      }

      switch (h->data_type) {	
      case (ISMRMRD::ISMRMRD_USHORT):
	return this->process(hmi, AsContainerMessage< hoNDArray< uint16_t > >(hmi->cont()), mmb);
	break;
      case (ISMRMRD::ISMRMRD_SHORT):
	return this->process(hmi, AsContainerMessage< hoNDArray< int16_t > >(hmi->cont()), mmb);
	break;
      case (ISMRMRD::ISMRMRD_UINT):
	return this->process(hmi, AsContainerMessage< hoNDArray< uint32_t > >(hmi->cont()), mmb);
	break;
      case (ISMRMRD::ISMRMRD_INT):
	return this->process(hmi, AsContainerMessage< hoNDArray< int32_t > >(hmi->cont()), mmb);
	break;
      case (ISMRMRD::ISMRMRD_FLOAT):
	return this->process(hmi, AsContainerMessage< hoNDArray< float > >(hmi->cont()), mmb);
	break;
      case (ISMRMRD::ISMRMRD_DOUBLE):
	return this->process(hmi, AsContainerMessage< hoNDArray< double > >(hmi->cont()), mmb);
	break;
      case (ISMRMRD::ISMRMRD_CXFLOAT):
	return this->process(hmi, AsContainerMessage< hoNDArray< std::complex<float> > >(hmi->cont()), mmb);
	break;
      case (ISMRMRD::ISMRMRD_CXDOUBLE):
	return this->process(hmi, AsContainerMessage< hoNDArray< std::complex<double> > >(hmi->cont()), mmb);
	break;
      default:
	GERROR("Unknown image data_type %d received\n", h->data_type);
	hmi->release();
	return GADGET_FAIL;
	break;
      }
    }
    return GADGET_OK;
  }
  
  GADGET_FACTORY_DECLARE(PythonGadget)
}
