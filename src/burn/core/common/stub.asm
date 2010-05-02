        .686
        .model  flat

DecodePointerInternal   proto   stdcall :PTR
EncodePointerInternal   proto   stdcall :PTR

        .const

        public  __imp__DecodePointer@4
__imp__DecodePointer@4  dword   DecodePointerInternal

        public  __imp__EncodePointer@4
__imp__EncodePointer@4  dword   EncodePointerInternal

        end
