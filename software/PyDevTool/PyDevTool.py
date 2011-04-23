#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, struct
from PyQt4.QtGui import *
from PyQt4.QtCore import *
from MainWindow import Ui_MainWindow
import serial

class PyDevTool (QMainWindow, Ui_MainWindow):
  
  parent = None
  serial = None
  serialTimer = None
  serialCount = 0
  serialState = {'command': None, 'length': 0, 'data': '', 'state': 0}
  
  def __init__(self, parent=None):
    QMainWindow.__init__(self)

    self.parent = parent
    
    self.setupUi(self)
    
    self.serialInit()
    
  def serialInit(self):
    self.serial = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
    self.serialTimer = QTimer(self)
    self.serialTimer.setInterval(1)
    self.connect(self.serialTimer, SIGNAL("timeout()"), self.serialTimeout)
    self.connect(self, SIGNAL("serialDataAvailable()"), self.serialRxHandler)
    self.serialTimer.start()
    
  def serialTimeout(self):
    self.serialCount += 1
    if self.serial.inWaiting() > 0:
      self.emit(SIGNAL("serialDataAvailable()"))
    if self.serialCount >= 40:
      self.serialCount = 0
      if self.AccGetData.isEnabled():
        self.serial.write(chr(16))

    
  def serialRxHandler(self):
    while self.serial.inWaiting() > 0:
      if self.serialState['state'] == 0:
        self.serialState['command'] = struct.unpack('B', self.serial.read())[0]
        self.serialState['state'] = 1
      elif self.serialState['state'] == 1:
        self.serialState['length'] = struct.unpack('B', self.serial.read())[0]
        self.serialState['state'] = 2
      elif self.serialState['state'] == 2:
        self.serialState['data'] += self.serial.read()
        if len(self.serialState['data']) == self.serialState['length']:
          self.serialHandleCommand()
          self.serialState['state'] = 0
          self.serialState['data'] = ''
        elif len(self.serialState['data']) > self.serialState['length']:
          self.close()

  def serialHandleCommand(self):
    cmd = self.serialState['command']
    data = self.serialState['data']
    
    if cmd == 16:
      data = struct.unpack('bbb', data) # unpack as signed word.
      self.emit(SIGNAL("AccXDataChanged(int)"), data[0])
      self.emit(SIGNAL("AccYDataChanged(int)"), data[1])
      self.emit(SIGNAL("AccZDataChanged(int)"), data[2])
  
  def MotorsSet(self, motor, value):
    self.serial.write(chr(8+motor))
    self.serial.write(chr(value))
  
  def MotorsSetFront(self, value):
    self.MotorsSet(0, value);
  
  def MotorsSetBack(self, value):
    self.MotorsSet(1, value);
    
  def MotorsSetLeft(self, value):
    self.MotorsSet(2, value);
    
  def MotorsSetRight(self, value):
    self.MotorsSet(3, value);
  
      
    
    
if __name__ == "__main__":
  app = QApplication([])
  QApplication.setOrganizationName("Quadrocopter")
  QApplication.setOrganizationDomain("nochwer.de")
  QApplication.setApplicationName("PyDevTool")
  QApplication.setApplicationVersion("0.0.1")
  app.maindialog = PyDevTool(app)
  app.setActiveWindow(app.maindialog)
  app.maindialog.show()
  sys.exit(app.exec_())