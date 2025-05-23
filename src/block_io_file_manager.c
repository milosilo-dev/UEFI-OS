#ifndef BI_File_Manager_C
#define BI_File_Manager_C
#include "../main.c" 

EFI_STATUS bi_file_manager(){
    EFI_STATUS status;

    EFI_STOP->SetAttribute(EFI_STOP, EFI_TEXT_ATTR(EFI_WHITE,EFI_BLACK));
    EFI_STOP->ClearScreen(EFI_STOP);

    EFI_GUID bio_guid = EFI_BLOCK_IO_PROTOCOL_GUID;
    EFI_BLOCK_IO_PROTOCOL *biop;
    UINTN num_handles = 0;
    EFI_HANDLE *handle_buffer = NULL;

    // LIP
    EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *lip;

    status = bs->OpenProtocol(image_handle, &lip_guid, (VOID **)&lip, image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if (EFI_ERROR(status)){
        printf(EFI_STOP, u"Can't open the Load Image Protocol. Error %d", status);

        bs->CloseProtocol(image_handle, &lip_guid, image_handle, NULL);
        return status;
    }

    status = bs->OpenProtocol(lip->DeviceHandle, &bio_guid, (VOID **)&biop, image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (EFI_ERROR(status)) {
        printf(EFI_STOP, u"Could not open Block IO Protocol for this loaded image, Error %d", status);
        return status;
    }

    UINT32 this_image_media_id = biop->Media->MediaId;

    bs->CloseProtocol(lip->DeviceHandle, &bio_guid, image_handle, NULL);
    bs->CloseProtocol(image_handle, &lip_guid, image_handle, NULL);

    // Block IO
    status = bs->LocateHandleBuffer(ByProtocol, &bio_guid, NULL, &num_handles, &handle_buffer);
    if (EFI_ERROR(status)) {
        printf(EFI_STOP, u"Could not locate Block IO Protocol, Error %d", status);
        return status;
    }

    UINT32 last_media_id = -1;

    EFI_STOP->SetCursorPosition(EFI_STOP, 3, 2);
    for (UINTN i = 0; i < num_handles; i++){
        status = bs->OpenProtocol(handle_buffer[i], &bio_guid, (VOID **)&biop, image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

        if (EFI_ERROR(status)) {
            printf(EFI_STOP, u"Could not open Block IO Protocol, Error %d", status);
            return status;
        }

        if (last_media_id != biop->Media->MediaId){
            printf(EFI_STOP, u"[NEW DISK, %d]  ", biop->Media->MediaId);
            if (biop->Media->MediaId == this_image_media_id)
                printf(EFI_STOP, u"[THIS DISK]");
            printf(EFI_STOP, u"\n\r");
        }
        
        printf(EFI_STOP, u"[NEW BLOCK] Rmv: %t, Pr: %t, lglPrt: %t, RdOnly: %t, Wrt$: %t, BlkSz: %d, IOAln %d, "
            u"LstBlk: %d, LwLBA: %d, LgbPerPhys: %d OptTrnLenGran: %d\r\n",
            biop->Media->RemovableMedia,
            biop->Media->MediaPresent,
            biop->Media->LogicalPartition,
            biop->Media->ReadOnly,
            biop->Media->WriteCaching,
            biop->Media->BlockSize,
            biop->Media->IoAlign,
            biop->Media->LastBlock,
            biop->Media->LowestAlignedLba,
            biop->Media->LogicalBlocksPerPhysicalBlock,
            biop->Media->OptimalTransferLengthGranularity);
        
        printf(EFI_STOP, u"\n\r");

        last_media_id = biop->Media->MediaId;
        bs->CloseProtocol(handle_buffer[i], &bio_guid, image_handle, NULL);
    }

    EFI_STOP->SetAttribute(EFI_STOP, EFI_TEXT_ATTR(EFI_YELLOW,EFI_BLACK));
    EFI_STOP->OutputString(EFI_STOP, u"\n\r");
    EFI_STOP->SetAttribute(EFI_STOP, EFI_TEXT_ATTR(EFI_YELLOW,EFI_RED));
    EFI_STOP->OutputString(EFI_STOP, u"Press any key to go back...");

    UINTN Index;
    EFI_EVENT WaitList[1];
    WaitList[0] = EFI_STIP->WaitForKey;
    bs->WaitForEvent(1, WaitList, &Index);

    return EFI_SUCCESS;
}

#endif