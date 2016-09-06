
#define EVENT_BUTTON_CLICK  0x1010
#define EVENT_COMBOBOX_SEL_INDEX_CHANGED 0X1090
#define EVENT_LISTBOX_SEL_INDEX_CHANGED 0X1080
#define SetValue(X,V) X=V
int x = 0;
int y = 0;

enum list
{
	HMUI_1=20001,
	ButtonControl_1=1,
	ButtonControl_2=2,
	ButtonControl_3=3,
	ButtonControl_4=4,
	ButtonControl_5=5,
	ListBoxControl_6=6,
	ComboBoxControl_7=7,
	TimerControl_8=8,
	SerialPortControl_9=9,
	HMUI_2=20002,
};

int InitializeControls()
{
	devi_init_serial_port(SerialPortControl_9,1,9600);

	return 0;
}

int ProcessEvent(int senderId, int event, char *data, int len)
{
	int iRet = 0;
    switch (event)
    {
    case EVENT_BUTTON_CLICK:
    {

		if(senderId == ButtonControl_1)
		{
			if( x == 1)
			{
				uie_ctrl_visual(ButtonControl_2);			
				char value1[1] = {0x12};
				devi_serial_port_write_data(SerialPortControl_9,value1,1);

			}
		uie_ctrl_unvisual(ButtonControl_1);
		}
		if(senderId == ButtonControl_3)
		{
			if( y == 3)
			{
				uie_ctrl_enable(ButtonControl_4);
			}
	
		}
    }
    break;

    case EVENT_LISTBOX_SEL_INDEX_CHANGED:
    {
        int SelectedIndex = 0;
        memcpy(&SelectedIndex,data + 15,sizeof(int));

		if(senderId == ListBoxControl_6)
		{
			SetValue(x,SelectedIndex);
		}
    }
    break;

    case EVENT_COMBOBOX_SEL_INDEX_CHANGED:
    {
        int SelectedIndex = 0;
        memcpy(&SelectedIndex,data + 15,sizeof(int));

		if(senderId == ComboBoxControl_7)
		{
			SetValue(y,SelectedIndex);
		}
    }
    break;

     default:
			break;
	}

	return 0;
}

int ReleaseControls()
{
	devi_release_serial_port(SerialPortControl_9);
	return 0;
}
