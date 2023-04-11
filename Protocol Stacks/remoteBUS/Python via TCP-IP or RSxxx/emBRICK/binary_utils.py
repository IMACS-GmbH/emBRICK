'''
File contains helper functions for binary Operations
'''

def convert16bitTo8bitList(list16bit):
    """
    Convert a list of 16-bit integers into a list of 8-bit integers.

    Args:
        list16bit (list): A list of 16-bit integers.

    Returns:
        list: A list of 8-bit integers.

    Example:
        >>> convert16bitTo8bitList([0x1234, 0xabcd])
        [18, 52, 171, 205]
    """
    list8bit = []
    for value in list16bit:
        a = (value >> 8) & 0xff
        b = value & 0xff
        list8bit.append(a)
        list8bit.append(b)
    return list8bit
    
    
def convert8bitTo16bitList(list8bit):
    """
    Convert a list of 8-bit integers into a list of 16-bit integers.

    If the input list has an odd number of elements, a zero is appended to the end
    to make it even.

    Args:
        list8bit (list): A list of 8-bit integers.

    Returns:
        list: A list of 16-bit integers.

    Example:
        >>> convert8bitTo16bitList([18, 52, 171, 205])
        [0x1234, 0xabcd]
    """
    list16bit = []
    if len(list8bit) % 2:
        list8bit.append(0)
    for i in range(0, len(list8bit), 2):
        byte0 = list8bit[i] << 8
        byte1 = list8bit[i+1]
        byte = byte0 + byte1
        list16bit.append(byte)
    return list16bit