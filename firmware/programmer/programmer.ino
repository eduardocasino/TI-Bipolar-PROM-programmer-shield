
// Firmware for the National/TI Bipolar PROM programmer shield
//
// (C) 2024 Eduardo Casino
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the “Software”), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// Commands:  Get version          "V"
//            Read whole PROM:     "R <CHIPNO>\n"
//            Read byte:           "r <CHIPNO> <ADDR>\n"
//            Write byte:          "w <CHIPNO> <ADDR> <BYTE>\n"
//            Simulate write byte: "s <CHIPNO> <ADDR> <BYTE>\n"
//            Blank check:         "K <CHIPNO>\n"
//            Excute test          "t <CHIPNO> <TEST_NUM> <TEST_PARAM>\n"
//                 0    Power test. Params:
//                          0  Power off
//                          1  Power on
//                          2  Set 10.5V
//                          3  Set 5V
//                 1    Chip enable. Params:
//                          0  Disable
//                          1  Enable
//                 2    Set address. Parameter is address number in hex
//                 3    Set program bit to ground. Parameter is bit mask
//
// Where:     <CHIPNO> is 0 for 740/741 and 1 for 742/743
//            <ADDR> is a three digit hex number in ascii
//            <BYTE> is a two digit hex number in ascii
//
// (V)ersion returns the version string, followed by "\r\nR\r\n"
// (R)ead prom returns the number of bytes read in hex, followed by "\r\n", followed
//        by data as an string of 2-byte hex digits, followed by "\r\nR\r\n"
// (r)ead returns the hex value, followed by "\r\nR\r\n"
// (w)rite returns the read hex value after programming, followed by "\r\nR\r\n"
// (s)imulate write returns the hex value that would have resulted after programming,
//        followed by "\r\nR\r\n"
// Blan(K) test returns the last blank address read in hex (or the memory size if all
//        blank), followed by "\r\nR\r\n"
//

// Address bus on port A:
// ADA - PA0 - Pin 22
// ADB - PA1 - Pin 23
// ADC - PA2 - Pin 24
// ADD - PA3 - Pin 25
// ADE - PA4 - Pin 26
// ADF_ADG - PA5 - Pin 27
// ADG_ADH - PA6 - Pin 28
// ADH_ADI - PA7 - Pin 29

// S2 / Extended address bus for 74s472/473 on port C (PC7)
// S2_ADF - PC7 - Pin 30

// Programming data bus on port L
// PB0 - PL0 - Pin 49
// PB1 - PL1 - Pin 48
// PB2 - PL2 - Pin 47
// PB3 - PL3 - Pin 46
// PB4 - PL4 - Pin 45
// PB5 - PL5 - Pin 44
// PB6 - PL6 - Pin 43
// PB7 - PL7 - Pin 42
  
// Reading data bus on port F
// DO0 - PF0 - Pin A0
// DO1 - PF1 - Pin A1
// DO2 - PF2 - Pin A2
// DO3 - PF3 - Pin A3
// DO4 - PF4 - Pin A4
// DO5 - PF5 - Pin A5
// DO6 - PF6 - Pin A6
// DO7 - PF7 - Pin A7

// Control signals on port K
// 5V/105V - PK0 - Pin A8
// VCC_EN  - PK1 - Pin A9
// ~S1     - PK2 - Pin A10

#define VERSION               "V010000"
#define PROG_PULSE_LENGTH      5     // In ms. Datasheet guarantees bit is programmed with a 0.9ms pulse, max 10ms
#define PROG_COOLING_DELAY    15     // In ms. Duty cycle is 25% nominal, 35% max, we go nominal

#define VCC_EN    A9
#define VCC_10V5  A8
#define S1        A10
#define S2        30

typedef enum { CHIP_256X8 = 0, CHIP_512X8, NUM_CHIPS } chip_type_t;
typedef enum { ST_ANY = 0, ST_READY, ST_WAIT_CHIP, ST_WAIT_ADDR, ST_WAIT_VALUE, ST_WAIT_TESTNO, ST_WAIT_TEST_PARAMS, ST_EXEC } state_t;

typedef struct {
  byte command;
  chip_type_t chip;               // Chip to program
  word address;                   // Address to read from / write to. Doubles as test parameters for the test command
  byte value;                     // Value to write / verify. Doubles as test number for the test command
} cmd_data_t;

typedef state_t (*state_fn_t)( cmd_data_t *cmd_data, state_t state, state_t next );

typedef struct {
  char command;
  state_t state;
  state_fn_t function;
  state_t next_state;
} st_machine_t;

state_t print_version( cmd_data_t *cmd_data, state_t unused1, state_t unused2 );

state_t get_command( cmd_data_t *cmd_data, state_t unused, state_t next );
state_t get_chip( cmd_data_t *cmd_data, state_t unused, state_t next );
state_t get_address( cmd_data_t *cmd_data, state_t unused, state_t next );
state_t get_value( cmd_data_t *cmd_data, state_t unused, state_t next );

state_t exec_read_prom( cmd_data_t *cmd_data, state_t unused1, state_t unused2 );
state_t exec_blank_check( cmd_data_t *cmd_data, state_t unused1, state_t unused2 );
state_t exec_read_prom_byte( cmd_data_t *cmd_data, state_t unused1, state_t unused2 );
state_t exec_write_prom_byte( cmd_data_t *cmd_data, state_t unused1, state_t unused2 );
state_t exec_simul_write_prom_byte( cmd_data_t *cmd_data, state_t unused1, state_t unused2 );
state_t exec_test( cmd_data_t *cmd_data, state_t unused1, state_t unused2 );

// State machine table
const st_machine_t machine[] = {

  { '*', ST_READY, get_command, ST_WAIT_CHIP },
  { 'V', ST_ANY, print_version, ST_READY },
  { 'R', ST_WAIT_CHIP, get_chip, ST_EXEC },
  { 'R', ST_EXEC, exec_read_prom, ST_READY },
  { 'K', ST_WAIT_CHIP, get_chip, ST_EXEC },
  { 'K', ST_EXEC, exec_blank_check, ST_READY },
  { 'r', ST_WAIT_CHIP, get_chip, ST_WAIT_ADDR },
  { 'r', ST_WAIT_ADDR, get_address, ST_EXEC },
  { 'r', ST_EXEC, exec_read_prom_byte, ST_READY },
  { 'w', ST_WAIT_CHIP, get_chip, ST_WAIT_ADDR },
  { 'w', ST_WAIT_ADDR, get_address, ST_WAIT_VALUE },
  { 'w', ST_WAIT_VALUE, get_value, ST_EXEC },
  { 'w', ST_EXEC, exec_write_prom_byte, ST_READY },
  { 's', ST_WAIT_CHIP, get_chip, ST_WAIT_ADDR },
  { 's', ST_WAIT_ADDR, get_address, ST_WAIT_VALUE },
  { 's', ST_WAIT_VALUE, get_value, ST_EXEC },
  { 's', ST_EXEC, exec_simul_write_prom_byte, ST_READY },
#ifdef TEST
  { 't', ST_WAIT_CHIP, get_chip, ST_WAIT_TESTNO },
  { 't', ST_WAIT_TESTNO, get_value, ST_WAIT_TEST_PARAMS },
  { 't', ST_WAIT_TEST_PARAMS, get_address, ST_EXEC },
  { 't', ST_EXEC, exec_test, ST_READY },
#endif   
  { 0 }
};

const unsigned int chip_sizes[NUM_CHIPS] = { 256, 512 };

inline void set_address( chip_type_t chip_type, unsigned int address ) __attribute__( ( always_inline ) );
void set_address( chip_type_t chip_type, unsigned int address )
{
  if ( CHIP_256X8 == chip_type )
  {
    PORTA = address;
  }
  else
  {
    PORTA = ( address & B00011111 ) | ( ( address >> 1 ) & B11100000 );
    PORTC = ( PORTC   & B01111111 ) | ( ( address << 2 ) & B10000000 );
  }
}

inline void enable_10V5( void ) __attribute__( ( always_inline ) );
void enable_10V5( void )
{
  digitalWrite( VCC_10V5, HIGH );
}

inline void disable_10V5( void ) __attribute__( ( always_inline ) );
void disable_10V5( void )
{
  digitalWrite( VCC_10V5, LOW );
}

inline void _power_on( void ) __attribute__( ( always_inline ) );
void _power_on( void )
{
  // Power on chip
  digitalWrite( VCC_EN, LOW );
} 

inline void power_on( void ) __attribute__( ( always_inline ) );
void power_on( void )
{
  // Ensure VCC is set to 5V
  disable_10V5();

  _power_on();
}

inline void power_off( void ) __attribute__( ( always_inline ) );
void power_off( void )
{
  digitalWrite( VCC_EN, HIGH );
}

inline void output_enable( chip_type_t chip_type ) __attribute__( ( always_inline ) );
void output_enable( chip_type_t chip_type )
{
  if ( CHIP_256X8 == chip_type )
  {
    digitalWrite( S2, LOW );
  }
  digitalWrite( S1, LOW );
}

inline void output_disable( chip_type_t chip_type ) __attribute__( ( always_inline ) );
void output_disable( chip_type_t chip_type )
{
  if ( CHIP_256X8 == chip_type )
  {
    digitalWrite( S2, HIGH );
  }
  digitalWrite( S1, HIGH );
}

inline void ground_pins( byte mask ) __attribute__( ( always_inline ) );
void ground_pins( byte mask )
{
  PORTL = mask;
}

inline void pullup_pins( byte mask ) __attribute__( ( always_inline ) );
void pullup_pins( byte mask )
{
  PORTL &= ~mask;
}

inline void prog_bit( chip_type_t chip_type, byte mask ) __attribute__( ( always_inline ) );
void prog_bit( chip_type_t chip_type, byte mask )
{
  // 1. Connect each output not being programmed to 5 V through 3K9 and apply the voltage
  //    specified in the table to the output to be programmed
  ground_pins( mask );
  
  // 2. Step Vcc to 10.5 V nominal
  enable_10V5();

  // 3. Apply a low-logic-level voltage to the chip-select input(s). This should occur between
  //    10 us and 1 ms after Vcc has reached its 10V5 level.
  delayMicroseconds( 100 );
  output_enable( chip_type );

  // 4. After the X pulse time is reached, a high logic level is applied to the chip-select
  //    inputs to disable the outputs. 
  delay( PROG_PULSE_LENGTH );
  output_disable( chip_type );

  // 5. Within 10 us to 1 ms after the chip-select input(s) reach a high logic level,
  //    Vcc should be stepped down to 5V, at which level verification can be accomplished.
  disable_10V5();
  pullup_pins( mask );
}

// Returns true if the bit is programmed
inline bool read_bit( chip_type_t chip_type, byte mask ) __attribute__( ( always_inline ) );
bool read_bit( chip_type_t chip_type, byte mask )
{
  bool value;
  output_enable( chip_type );
  asm ("nop\n\t");                    // 62.5ns delay, just in case
  value = PINF & mask;
  output_disable( chip_type );

  return value;
}

inline state_t _set_st_ready( void ) __attribute__( ( always_inline ) );
state_t _set_st_ready( void )
{
  Serial.println( "R" );

  return ST_READY;
}

inline state_t set_st_ready( void ) __attribute__( ( always_inline ) );
state_t set_st_ready( void )
{
  power_off();

  return _set_st_ready();
}

inline state_t _set_st_error( void ) __attribute__( ( always_inline ) );
state_t _set_st_error( void )
{
  Serial.println( "E" );

  return ST_READY;
}

inline state_t set_st_error( void ) __attribute__( ( always_inline ) );
state_t set_st_error( void )
{
  power_off();

  return _set_st_error();
}

void wait_first_non_blank( void )
{
  byte char_in = ' ';

  while ( isSpace( char_in ) )
  {
    while ( !Serial.available() )
      ;
    if ( isSpace( char_in = Serial.peek() ) )
    {
      Serial.read();
    }
  }
}

long int get_hex( void )
{
  byte digit;
  long int hex = 0;

  wait_first_non_blank();

  do
  {
    while ( !Serial.available() )
      ;
    digit = Serial.read();

    if ( isHexadecimalDigit( digit ) )
    {
      hex <<= 4;

      if ( digit >= '0' && digit <= '9' )
      {
        hex += digit - '0';
      }
      else if ( digit >= 'A' && digit <= 'F' )
      {
        hex += digit - 'A' + 10;
      }
      else if ( digit >= 'a' && digit <= 'f' )
      {
        hex += digit - 'a' + 10;
      }
    }
    else if ( !isSpace( digit ) )
    {
      return -1;
    }
  } while ( !isSpace( digit ) );

  return hex;
}

state_t get_value( cmd_data_t *cmd_data, state_t unused, state_t next )
{
  long int value = get_hex();

  if (value < 0 || value > 0xFF )
  {
    return set_st_error();
  }

  cmd_data->value = (byte) value;

  return next; 
}

state_t get_address( cmd_data_t *cmd_data, state_t unused, state_t next )
{
  long int address = get_hex();

  if ( address < 0 || address >= chip_sizes[cmd_data->chip] )
  {
    return set_st_error();
  }

  cmd_data->address = (word) address;

  return next; 
}

state_t get_command( cmd_data_t *cmd_data, state_t unused, state_t next )
{
  int c;

  wait_first_non_blank();

  cmd_data->command = Serial.read();

  for ( c = 0; machine[c].command != cmd_data->command; ++c )
    ;
    
  if ( machine[c].command == '\0' )
  {
    return set_st_error();
  }
    
  return next;
}

state_t get_chip( cmd_data_t *cmd_data, state_t unused, state_t next )
{
  wait_first_non_blank();

  char chip_c = Serial.read();

  if ( chip_c != '0' && chip_c != '1' )
  {
     return set_st_error();
  }

  cmd_data->chip = chip_c - '0';

  return next;
}

byte read_prom_byte( chip_type_t chip_type, unsigned int address )
{
  byte value;

  set_address( chip_type, address );
  output_enable( chip_type );
  asm ("nop\n\t");                    // 62.5ns delay, just in case
  value = PINF;
  output_disable( chip_type );
  return value;
}

#ifdef TEST
state_t exec_test( cmd_data_t *cmd_data, state_t unused1, state_t unused2 )
{
  unsigned long start, now;
  byte mask;

  switch ( cmd_data->value )          // Remember: 'value' doubles as test number
  {
    case 0:                           // Power tests
      switch ( cmd_data->address )    // Remember: 'address' doubles as test parameter
      {
        case 0:                       // Power off
          Serial.println( "Powering off" );
          power_off();
          break;
        
        case 1:                       // Power on
          Serial.println( "Powering on" );
          _power_on();                // Powers on without disabling 10V5 (if set)
          break;

        case 2:                       // Set 10V5
          Serial.println( "Setting 10V5" );
          enable_10V5();
          break;
        
        case 3:                       // Set 5V
          Serial.println( "Setting 5V" );
          disable_10V5();
          break;
        
        default:                      // Invalid parameter
          Serial.println( "Invalid test parameter" );
          return _set_st_error();     // Set ST_ERROR without powering off
      }
      break;
    
    case 1:                           // Chip enable/disable tests
      switch ( cmd_data->address )    // Remember: 'address' doubles as test parameter
      {
        case 0:                       // Disable chip
          Serial.println( "Disabling chip output" );
          output_disable( cmd_data->chip );
          break;
        
        case 1:                       // Enable chip
          Serial.println( "Enabling chip output" );
          output_enable( cmd_data->chip );
          break;

        default:                      // Invalid parameter
          Serial.println( "Invalid test parameter" );
          return _set_st_error();     // Set ST_ERROR without powering off
      }
      break;

    case 2:                           // Address selection test
      Serial.print( "Setting address " );
      Serial.println( cmd_data->address, HEX );
      set_address( cmd_data->chip, cmd_data->address );
      break;

    case 3:                           // Data pin grounding test
      mask = (byte) cmd_data->address; // Remember: 'address' doubles as test parameter
      Serial.print( "Grounding pins " );
      Serial.println( mask, BIN );
      ground_pins( mask );
      break;

    default:                          // Invalid test
      Serial.println( "Invalid test number" );
      return _set_st_error();         // Set ST_ERROR without powering off
  }

  return _set_st_ready();             // Set ST_READY without powering off
}
#endif

state_t write_prom_byte( cmd_data_t *cmd_data, bool do_write )
{
  byte existing, returned, mask;
  bool programmed;
  
  power_on();

  existing = read_prom_byte( cmd_data->chip, cmd_data->address );    // This also set the address bus
  returned = existing;

  if ( existing != cmd_data->value )
  {
    for ( int bit = 0; bit < 8; ++bit )
    {
      mask = 1 << bit;

      // Check every bit to see if it needs programming
      if ( (existing & mask) != (cmd_data->value & mask) )
      {
        // Bits are different
        if ( ! (existing & mask) )
        {
          // They are different, but it is not programmed. Okay, burn it
          // Address was already set by read_prom_byte()
          //
          if ( do_write )
          {
            _power_on();

            prog_bit( cmd_data->chip, mask );
            delayMicroseconds( 10 );
            // Verify that bit is programmed
            programmed = read_bit( cmd_data->chip, mask );
      
            power_off();

            delay( PROG_COOLING_DELAY );

            if ( ! programmed )
            {
              // Stop here, there was a problem programming the last bit
              break;
            }
          }

          returned |= mask;
        }
        else
        {
          // Stop there, because bits are different and already programmed
          break;
        }
      }
    }
  }
  
  // Return programmed value
  Serial.println( returned, HEX );

  return set_st_ready();
}

state_t exec_write_prom_byte( cmd_data_t *cmd_data, state_t unused1, state_t unused2 )
{
  return write_prom_byte( cmd_data, true );
}

state_t exec_simul_write_prom_byte( cmd_data_t *cmd_data, state_t unused1, state_t unused2 )
{
  return write_prom_byte( cmd_data, false );
}

state_t exec_read_prom_byte( cmd_data_t *cmd_data, state_t unused1, state_t unused2 )
{
  power_on();

  byte data = read_prom_byte( cmd_data->chip, cmd_data->address ); 
    
  Serial.println( data, HEX );
  
  return set_st_ready();
}

state_t exec_read_prom( cmd_data_t *cmd_data, state_t unused1, state_t unused2 )
{
  unsigned int address;
  byte data;

  Serial.println( chip_sizes[cmd_data->chip], HEX );

  power_on();

  for ( address = 0; address < chip_sizes[cmd_data->chip]; ++address )
  {
    data = read_prom_byte( cmd_data->chip, address ); 
    Serial.print( data < 16 ? "0" : "");
    Serial.print( data, HEX );
  }
  Serial.println( "" );

  return set_st_ready();
}

state_t exec_blank_check( cmd_data_t *cmd_data, state_t unused1, state_t unused2 )
{
  unsigned int address;
  byte data;

  power_on();
  
  for ( address = 0; address < chip_sizes[cmd_data->chip]; ++address )
  {
    if ( 0 != ( data = read_prom_byte( cmd_data->chip, address ) ) )
    {
      break;
    }
  }
  Serial.println( address, HEX );

  return set_st_ready();
}

state_t print_version( cmd_data_t *cmd_data, state_t unused1, state_t unused2 )
{
  Serial.println( VERSION );

  return set_st_ready();
}

state_t programmer_init( cmd_data_t *cmd_data )
{
  memset( cmd_data, 0, sizeof( cmd_data_t ) );
  
  return set_st_ready();
}

void setup( void )
{

  // Set pin directions
  //
  DDRA =  0xFF;       // Set address bus to output
  DDRC |= B10000000;  // Set extended address pin / S2 to output
  DDRF =  0;          // Set reading data bus to input
  DDRL =  0xFF;       // Set programming data bus to output
  DDRK |= B00000111;  // Set control signals to output

  // Ensure pull-ups are disabled for PORTF
  PORTF = 0;
  
  power_off();
  disable_10V5();
  ground_pins( 0 );
  output_disable( CHIP_256X8 );
  output_disable( CHIP_512X8 );

  Serial.begin( 57600 );
}

void loop( void )
{
  int index;
  cmd_data_t cmd_data;
  state_t state = programmer_init( &cmd_data );

  while ( true )
  {
    if ( Serial.available() > 0 )
    {
      char next_char = Serial.peek();

      if ( isSpace( next_char ) )
      {
        // Just ignore
        Serial.read();
        continue;
      }
    }

    for ( index = 0; 0 != machine[index].command; ++index )
    {
      if ( ( machine[index].command == '*' || machine[index].command == cmd_data.command )
          && ( machine[index].state == state || machine[index].state == ST_ANY ) )
      {
        state = machine[index].function( &cmd_data, state, machine[index].next_state );
        break;
      }
    }

    if ( 0 == machine[index].command )
    {
      state = set_st_error();
    }
  }
}
