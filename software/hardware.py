import serial
import struct
import time
import os

class Hardware:
  def __init__ (self, port: str):
    self.port = serial.Serial(port, baudrate=921600, timeout=0.1)
    self.buffer = bytearray(1_000_000)
  
  def crc_is_valid (self):
    crc = 0
    for tmp in self.buffer:
      tmp ^= crc & 0xff
      tmp ^= (tmp << 4) & 0xff
      crc = (((tmp << 8) | ((crc & 0xff00) >> 8)) ^ ((tmp >> 4) & 0xff) ^ (tmp << 3)) & 0xffff
    return crc == 0
  
  def get_packet(self):
    self.buffer.clear()
    
    self.port.flushOutput()
    self.port.flushInput()

    while (self.port.inWaiting() == 0):
      pass

    start = time.time()
    while time.time() - start < 0.4:
      data = self.port.read(64)
      self.buffer.extend(data)
      if len(data):
        start = time.time()

    if len(self.buffer) > 1:
      if not self.crc_is_valid():
        return False, self.buffer
    
    return True, self.buffer
  
  def bytes_to_samples(self, data):
    tmp = list(struct.unpack('<%dh' % (len(data) // 2), data))
    tmp.pop() # Remove CRC
    return tmp
  
  