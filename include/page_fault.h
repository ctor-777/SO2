
#ifndef __PAGE_FAULT_H__
#define __PAGE_FAULT_H__


void segmentation_fault_handler();

void segmentation_fault_service(unsigned int eip, unsigned int err);

#endif
