#
# Makefile for all task
#

include common.make

# hkkim
LOCAL_HOME = $(shell pwd)
NFS_HOME = $(shell pwd)/NFS

export LOCAL_HOME NFS_HOME
.PHONY : CLI WDT SCAN HOST LINK SCU SIM ICCP LIB EXIT 61850LIB NTP

# end 

all:
	@if [ -d lib ]; then $(CD) lib; $(MAKE) all; fi
	@if [ -d WDT ]; then $(CD) WDT; $(MAKE) all; fi
	@if [ -d CLI ]; then $(CD) CLI; $(MAKE) all; fi
	@if [ -d EXIT ]; then $(CD) EXIT; $(MAKE) all; fi
	@if [ -d SCAN ]; then $(CD) SCAN; $(MAKE) all; fi
	@if [ -d MPUIF ]; then $(CD) MPUIF; $(MAKE) all; fi
	@if [ -d SIM ]; then $(CD) SIM; $(MAKE) all; fi
	@if [ -d 61850Lib ]; then $(CD) 61850Lib; $(MAKE) all; fi
	@if [ -d 61850Client ]; then $(CD) 61850Client; $(MAKE) all; fi
	@if [ -d ntpClient ]; then $(CD) ntpClient; $(MAKE) all; fi


# hkkim
61850LIB:
	@if [ -d 61850Lib ]; then $(CD) 61850Lib; $(MAKE) all; fi

NTP:
	@if [ -d ntpClient ]; then $(CD) ntpClient; $(MAKE) clean; $(MAKE) all; fi

CLI: 
	@if [ -d CLI ]; then $(CD) CLI; $(MAKE) clean; $(MAKE) all; fi
EXIT: 
	@if [ -d EXIT ]; then $(CD) EXIT; $(MAKE) clean; $(MAKE) all; fi


LIB: 
	@if [ -d lib ]; then $(CD) lib; $(MAKE) all; fi
WDT: 	
	@if [ -d WDT ]; then $(CD) WDT; $(MAKE) all; fi

SCAN:	
	@if [ -d SCAN ]; then $(CD) SCAN; $(MAKE) all; fi
HOST:
	@if [ -d HOST ]; then $(CD) HOST; $(MAKE) all; fi
LINK :
	@if [ -d LINK ]; then $(CD) LINK; $(MAKE) all; fi
SCU : 	
	@if [ -d SCU ]; then $(CD) SCU; $(MAKE) all; fi
SIM :
	@if [ -d SIM ]; then $(CD) SIM; $(MAKE) all; fi
ICCP :
	@if [ -d ICCP ]; then $(CD) ICCP; $(MAKE) all; fi
	
# end 	

install:
	@if [ -d lib ]; then $(CD) lib; $(MAKE) install; fi
	@if [ -d WDT ]; then $(CD) WDT; $(MAKE) install; fi
	@if [ -d CLI ]; then $(CD) CLI; $(MAKE) install; fi
	@if [ -d SCAN ]; then $(CD) SCAN; $(MAKE) install; fi
	@if [ -d MPUIF ]; then $(CD) MPUIF; $(MAKE) install; fi
	@if [ -d SIM ]; then $(CD) SIM; $(MAKE) install; fi
	@if [ -d 61850Lib ]; then $(CD) 61850Lib; $(MAKE) install; fi
	@if [ -d 61850Client ]; then $(CD) 61850Client; $(MAKE) install; fi
	@if [ -d ntpClient ]; then $(CD) ntpClient; $(MAKE) install; fi

uninstall:
	@if [ -d lib ]; then $(CD) lib; $(MAKE) uninstall; fi
	@if [ -d WDT ]; then $(CD) WDT; $(MAKE) uninstall; fi
	@if [ -d CLI ]; then $(CD) CLI; $(MAKE) uninstall; fi
	@if [ -d SCAN ]; then $(CD) SCAN; $(MAKE) uninstall; fi
	@if [ -d MPUIF ]; then $(CD) MPUIF; $(MAKE) uninstall; fi
	@if [ -d SIM ]; then $(CD) SIM; $(MAKE) uninstall; fi
	@if [ -d 61850Lib ]; then $(CD) 61850Lib; $(MAKE) uninstall; fi
	@if [ -d 61850Client ]; then $(CD) 61850Client; $(MAKE) uninstall; fi
	@if [ -d ntpClient ]; then $(CD) ntpClient; $(MAKE) uninstall; fi

clean:
	@if [ -d lib ]; then $(CD) lib; $(MAKE) clean; fi
	@if [ -d WDT ]; then $(CD) WDT; $(MAKE) clean; fi
	@if [ -d CLI ]; then $(CD) CLI; $(MAKE) clean; fi
	@if [ -d SCAN ]; then $(CD) SCAN; $(MAKE) clean; fi
	@if [ -d MPUIF ]; then $(CD) MPUIF; $(MAKE) clean; fi
	@if [ -d SIM ]; then $(CD) SIM; $(MAKE) clean; fi
	@if [ -d 61850Lib ]; then $(CD) 61850Lib; $(MAKE) clean; fi
	@if [ -d 61850Client ]; then $(CD) 61850Client; $(MAKE) clean; fi
	@if [ -d ntpClient ]; then $(CD) ntpClient; $(MAKE) clean; fi

tags:
	@if [ -d lib ]; then $(CD) lib; $(MAKE) tags; fi
	@if [ -d WDT ]; then $(CD) WDT; $(MAKE) tags; fi
	@if [ -d CLI ]; then $(CD) CLI; $(MAKE) tags; fi
	@if [ -d SCAN ]; then $(CD) SCAN; $(MAKE) tags; fi
	@if [ -d MPUIF ]; then $(CD) MPUIF; $(MAKE) tags; fi
	@if [ -d SIM ]; then $(CD) SIM; $(MAKE) tags; fi
	@if [ -d 61850Lib ]; then $(CD) 61850Lib; $(MAKE) tags; fi
	@if [ -d 61850Client ]; then $(CD) 61850Client; $(MAKE) tags; fi
	@if [ -d ntpClient ]; then $(CD) ntpClient; $(MAKE) tags; fi


depend:
	@if [ -d lib ]; then $(CD) lib; $(MAKE) depend; fi
	@if [ -d WDT ]; then $(CD) WDT; $(MAKE) depend; fi
	@if [ -d CLI ]; then $(CD) CLI; $(MAKE) depend; fi
	@if [ -d SCAN ]; then $(CD) SCAN; $(MAKE) depend; fi
	@if [ -d MPUIF ]; then $(CD) MPUIF; $(MAKE) depend; fi
	@if [ -d SIM ]; then $(CD) SIM; $(MAKE) depend; fi
	@if [ -d 61850Lib ]; then $(CD) 61850Lib; $(MAKE) depend; fi
	@if [ -d 61850Client ]; then $(CD) 61850Client; $(MAKE) depend; fi
	@if [ -d ntpClient ]; then $(CD) ntpClient; $(MAKE) depend; fi

