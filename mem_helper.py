PHYS_MEM_BASE    = 0x80000000
DIRECT_MAP_BASE  = 0xFFFFFFC000000000

def PHYS_TO_VIRT(paddr):
    return paddr + DIRECT_MAP_BASE - PHYS_MEM_BASE

def VIRT_TO_PHYS(paddr):
    return paddr - DIRECT_MAP_BASE + PHYS_MEM_BASE

def vpn2(va):
    return (va >> 30) & 0x1FF

def vpn1(va):
    return (va >> 21) & 0x1FF

def vpn0(va):
    return (va >> 12) & 0x1FF

def resolve(va):
    print(f"VPN2: {vpn2(va)}")
    print(f"VPN1: {vpn1(va)}")
    print(f"VPN0: {vpn0(va)}")

resolve(0x80000000)
