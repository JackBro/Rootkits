The ZwCreateFile API is hooked and returns an error 
if the created file is "c:\pron.txt".


*/


#include <ntddk.h>

// usefull <3
#define SYSTEMSERVICE(_name)  KeServiceDescriptorTable.ServiceTable[*(DWORD *) ((unsigned char *)_name + 1)]

#define DEBUG
typedef unsigned long DWORD, *PDWORD;
typedef unsigned char BYTE, *PBYTE;


void hooking(void);
void Unhooking();
NTSTATUS DriverEntry(PDRIVER_OBJECT, PUNICODE_STRING); // main() du driver
void Unload_driver(IN PDRIVER_OBJECT);




#pragma pack(1)
typedef struct ServiceDescriptorEntry 
{
    PDWORD ServiceTable;
    PDWORD CounterTableBase;
    DWORD  ServiceLimit;
    PBYTE  ArgumentTable;
} SDT;
#pragma pack()

__declspec(dllimport) SDT KeServiceDescriptorTable;


typedef NTSYSAPI NTSTATUS (*ZWCREATEFILE)
(

    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    
);

ZWCREATEFILE OrigZwCreateFile;
extern ZWCREATEFILE OrigZwCreateFile;
/*
NTSTATUS RtlCompareUnicodeString(      

        IN PUNICODE_STRING String1, 
        IN PUNICODE_STRING String2, 
        IN BOOLEAN  CaseInSensitive  
    );
    



typedef struct _OBJECT_ATTRIBUTES {
    ULONG  Length;
    HANDLE  RootDirectory;
    PUNICODE_STRING  ObjectName; <= Yawn  :]
    ULONG  Attributes;
    PVOID  SecurityDescriptor;
    PVOID  SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
typedef CONST OBJECT_ATTRIBUTES *PCOBJECT_ATTRIBUTES;

*/


NTSTATUS FakeZwCreateFile(

                          OUT PHANDLE FileHandle,
                          IN ACCESS_MASK DesiredAccess,
                          IN POBJECT_ATTRIBUTES ObjectAttributes,
                          OUT PIO_STATUS_BLOCK IoStatusBlock,
                          IN PLARGE_INTEGER AllocationSize OPTIONAL,
                          IN ULONG FileAttributes,
                          IN ULONG ShareAccess,
                          IN ULONG CreateDisposition,
                          IN ULONG CreateOptions,
                          IN PVOID EaBuffer OPTIONAL,
                          IN ULONG EaLength) 
                          
{
                         
                        UNICODE_STRING FileName;
                        RtlInitUnicodeString(&FileName, L"\\??\\C:\\pron.txt");
                        
                        //KdPrint(("FILEZ : %wZ\n", ObjectAttributes->ObjectName)); // unicode string
                        
                        if(RtlCompareUnicodeString(ObjectAttributes->ObjectName,&FileName, TRUE)==0x00) 
                        {
                            DbgPrint("pron text file detected !\n");
                            return(STATUS_OBJECT_NAME_INVALID);
                        } 
                        
                        else 
                       {
    
                        // we call the real function
                         ((ZWCREATEFILE)(OrigZwCreateFile)) (
                        
                                                            FileHandle,
                                                            DesiredAccess,
                                                            ObjectAttributes,
                                                            IoStatusBlock,
                                                            AllocationSize,
                                                            FileAttributes,
                                                            ShareAccess,
                                                            CreateDisposition,
                                                            CreateOptions,
                                                            EaBuffer,
                                                            EaLength);
                                                            return(STATUS_SUCCESS);
                            //    ZwCreateFile returns STATUS_SUCCESS on success or an appropriate NTSTATUS error code on failure
                                                                 
                         }
}

void Hooking(void) {

    //cli, sti - disable/enable  interruptions
    _asm{cli}
    //  set the hook
    OrigZwCreateFile = (ZWCREATEFILE) (SYSTEMSERVICE(ZwCreateFile));
    (ZWCREATEFILE) (SYSTEMSERVICE(ZwCreateFile)) =    FakeZwCreateFile;
    _asm{sti}
    
}

void Unhooking() 
{

    _asm{cli}
    (ZWCREATEFILE) (SYSTEMSERVICE(ZwCreateFile)) = OrigZwCreateFile; // restore
    _asm{sti}
}

void Unload_driver(IN PDRIVER_OBJECT DriverObject) {
    
    DbgPrint("And now, unhooking API :] \n");
    Unhooking();
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING RegistryPath) {
    
    driverObject->DriverUnload  = Unload_driver;
    
    DbgPrint("Hooking API!\n");
    Hooking();
    
    return(STATUS_SUCCESS);
}