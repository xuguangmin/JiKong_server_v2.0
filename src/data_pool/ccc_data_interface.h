#ifndef __CCC_DATA_INTERFACE_H__
#define __CCC_DATA_INTERFACE_H__

typedef void (*data_interface_callback)(unsigned char *buffer, int size);

extern int data_interface_init(data_interface_callback callback);


#endif  /* __CCC_DATA_INTERFACE_H__ */
