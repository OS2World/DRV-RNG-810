; RPstatus bit values 

RPERR   =8000h              ;  error occurred, err in RPstatus    
RPDEV   =4000h              ;  error code defined by driver       
RPBUSY  =0200h              ;  device is busy                     
RPDONE  =0100h              ;  driver done with request packet    

; error codes returned in RPstatus 

ERROR_WRITE_PROTECT         =0000h
ERROR_BAD_UNIT              =0001h
ERROR_NOT_READY             =0002h
ERROR_BAD_COMMAND           =0003h
ERROR_CRC                   =0004h
ERROR_BAD_LENGTH            =0005h    
ERROR_SEEK                  =0006h
ERROR_NOT_DOS_DISK          =0007h
ERROR_SECTOR_NOT_FOUND      =0008h
ERROR_OUT_OF_PAPER          =0009h
ERROR_WRITE_FAULT           =000Ah
ERROR_READ_FAULT            =000Bh
ERROR_GEN_FAILURE           =000Ch
ERROR_DISK_CHANGE           =000Dh
ERROR_WRONG_DISK            =000Fh
ERROR_UNCERTAIN_MEDIA       =0010h
ERROR_CHAR_CALL_INTERRUPTED =0011h
ERROR_NO_MONITOR_SUPPORT    =0012h
ERROR_INVALID_PARAMETER     =0013h
ERROR_DEVICE_IN_USE         =0014h

;  driver device attributes word 

DAW_CHR    =8000h           ; 1=char, 0=block                     
DAW_IDC    =4000h           ; 1=IDC available in this DD          
DAW_IBM    =2000h           ; 1=non-IBM block format              
DAW_SHR    =1000h           ; 1=supports shared device access     
DAW_OPN    =0800h           ; 1=open/close, or removable media    
DAW_LEVEL1 =0080h           ; level 1                             
DAW_LEVEL2 =0100h           ; level 2 DosDevIOCtl2                
DAW_LEVEL3 =0180h           ; level 3 bit strip                   
DAW_GIO    =0040h           ; 1=generic IOCtl supported           
DAW_CLK    =0008h           ; 1=CLOCK device                      
DAW_NUL    =0004h           ; 1=NUL device                        
DAW_SCR    =0002h           ; 1=STDOUT (screen)                   
DAW_KBD    =0001h           ; 1=STDIN  (keyboard)                 

; capabilities bit strip 

CBS_SHD    =0001h           ; 1=shutdown/DevIOCtl2                
CBS_HMEM   =0002h           ; hign memory map for adapters        
CBS_PP     =0004h           ; supports parallel ports             


