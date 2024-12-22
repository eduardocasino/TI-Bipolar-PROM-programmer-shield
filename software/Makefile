#
# prom - A command-line utility to interface with the poor's man
#        National/TI Bipolar PROM Programmer
# 
# (C) 2024 Eduardo Casino
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the “Software”), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#

CC = gcc
LDFLAGS =
TARGET = prom
OBJ = prom.o options.o serial.o binfile.o ihex.o command.o files.o hexdump.o

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJ)

prom.o: globals.h options.h binfile.h ihex.h files.h command.h serial.h

options.o: globals.h options.h binfile.h ihex.h files.h command.h

serial.o: globals.h

binfile.o: globals.h files.h

ihex.o: globals.h files.h

command.o: globals.h files.h hexdump.h serial.h

files.o: globals.h files.h
