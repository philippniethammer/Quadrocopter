/* This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief This file shows how to implement a half duplex software driven uart.
 *
 * This single file shows how to implement a software uart using
 * Timer0 and external interrupt 0.
 *
 * Note that the RX_PIN must be the external interrupt 0 pin on
 * your AVR of choice. The TX_PIN can be chosen to be any suitable
 * pin. Note that this code is intended to run from an internal
 * 8 MHz clock source.
 * If other operating voltages and/or temperatures than 5 Volts and 25 Degrees Celsius
 * are desired, consider calibrating the internal oscillator.
 *
 * \par Application note:
 *      AVR304: Half Duplex Interrupt Driven Software UART
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 * \author
 *      Atmel Corporation: http://www.atmel.com \n
 *      Support email: avr@atmel.com
 *
 * $Name$
 * $Revision: 3778 $
 * $RCSfile$
 * $Date: 2008-04-11 15:05:31 +0200 (fr, 11 apr 2008) $  \n
 ******************************************************************************/
#include <avr/io.h>     // Device specifics.
#include <avr/interrupt.h>     // The __enable_interrupt() intrinsic.
#include <avr/sfr_defs.h>
#include "SUART.h"


//This section chooses the correct timer values for the chosen baudrate.
#ifdef  BR_9600
    #define TICKS2COUNT         103  //!< Ticks between two bits.
    #define TICKS2WAITONE       103  //!< Wait one bit period.
    #define TICKS2WAITONE_HALF  155  //!< Wait one and a half bit period.
#endif
#ifdef  BR_19200
    #define TICKS2COUNT          51  //!< Ticks between two bits.
    #define TICKS2WAITONE        51  //!< Wait one bit period.
    #define TICKS2WAITONE_HALF   77  //!< Wait one and a half bit period.
#endif
#ifdef  BR_38400
    #define TICKS2COUNT          25  //!< Ticks between two bits.
    #define TICKS2WAITONE        25  //!< Wait one bit period.
    #define TICKS2WAITONE_HALF   38  //!< Wait one and a half bit period.
#endif

#define INTERRUPT_EXEC_CYCL   9       //!< Cycles to execute interrupt rutine from interrupt.

//Some IO, timer and interrupt specific defines.

#if (defined(__ATmega8__) || defined(__AVR_ATmega8__))
  #define ENABLE_TIMER_INTERRUPT( )       ( TIMSK |= ( 1<< OCIE2 ) )
  #define DISABLE_TIMER_INTERRUPT( )      ( TIMSK &= ~( 1<< OCIE2 ) )
  #define CLEAR_TIMER_INTERRUPT( )        ( TIFR |= ((1 << OCF2) ) )
  #define ENABLE_EXTERNAL0_INTERRUPT( )   ( GICR |= ( 1<< INT0 ) )
  #define DISABLE_EXTERNAL0_INTERRUPT( )  ( GICR &= ~( 1<< INT0 ) )
  #define TX_PIN           PD3               //!< Transmit data pin
  #define RX_PIN           PD2               //!< Receive data pin, must be INT0
  #define TCCR             TCCR2             //!< Timer/Counter Control Register
  #define TCCR_P           TCCR2             //!< Timer/Counter Control (Prescaler) Register
  #define OCR              OCR2              //!< Output Compare Register
  #define EXT_IFR          GIFR              //!< External Interrupt Flag Register
  #define EXT_ICR          MCUCR             //!< External Interrupt Control Register
  #define TIMER_COMP_VECT  TIMER2_COMP_vect  //!< Timer Compare Interrupt Vector

#elif  (defined(__ATmega16__) || defined(__ATmega32__))
  #define ENABLE_TIMER_INTERRUPT( )       ( TIMSK |= ( 1<< OCIE0 ) )
  #define DISABLE_TIMER_INTERRUPT( )      ( TIMSK &= ~( 1<< OCIE0 ) )
  #define CLEAR_TIMER_INTERRUPT( )        ( TIFR |= ((1 << OCF0) ) )
  #define ENABLE_EXTERNAL0_INTERRUPT( )   ( GICR |= ( 1<< INT0 ) )
  #define DISABLE_EXTERNAL0_INTERRUPT( )  ( GICR &= ~( 1<< INT0 ) )
  #define TX_PIN           PD3               //!< Transmit data pin
  #define RX_PIN           PD2               //!< Receive data pin, must be INT0
  #define TCCR             TCCR0             //!< Timer/Counter Control Register
  #define TCCR_P           TCCR0             //!< Timer/Counter Control (Prescaler) Register
  #define OCR              OCR0              //!< Output Compare Register
  #define EXT_IFR          GIFR              //!< External Interrupt Flag Register
  #define EXT_ICR          MCUCR             //!< External Interrupt Control Register
  #define TIMER_COMP_VECT  TIMER0_COMP_vect  //!< Timer Compare Interrupt Vector

#elif defined(__ATmega128__)
  #define ENABLE_TIMER_INTERRUPT( )       ( TIMSK |= ( 1<< OCIE0 ) )
  #define DISABLE_TIMER_INTERRUPT( )      ( TIMSK &= ~( 1<< OCIE0 ) )
  #define CLEAR_TIMER_INTERRUPT( )        ( TIFR |= ((1 << OCF0) ) )
  #define ENABLE_EXTERNAL0_INTERRUPT( )   ( EIMSK |= ( 1<< INT0 ) )
  #define DISABLE_EXTERNAL0_INTERRUPT( )  ( EIMSK &= ~( 1<< INT0 ) )
  #define TX_PIN           PD1               //!< Transmit data pin
  #define RX_PIN           PD0               //!< Receive data pin, must be INT0
  #define TCCR             TCCR0             //!< Timer/Counter Control Register
  #define TCCR_P           TCCR0             //!< Timer/Counter Control (Prescaler) Register
  #define OCR              OCR0              //!< Output Compare Register
  #define EXT_IFR          EIFR              //!< External Interrupt Flag Register
  #define EXT_ICR          EICRA             //!< External Interrupt Control Register
  #define TIMER_COMP_VECT  TIMER0_COMP_vect  //!< Timer Compare Interrupt Vector

#elif defined(__ATmega169__)
  #define ENABLE_TIMER_INTERRUPT( )       ( TIMSK0 |= ( 1<< OCIE0A ) )
  #define DISABLE_TIMER_INTERRUPT( )      ( TIMSK0 &= ~( 1<< OCIE0A ) )
  #define CLEAR_TIMER_INTERRUPT( )        ( TIFR0 |= ((1 << OCF0A) ) )
  #define ENABLE_EXTERNAL0_INTERRUPT( )   ( EIMSK |= ( 1<< INT0 ) )
  #define DISABLE_EXTERNAL0_INTERRUPT( )  ( EIMSK &= ~( 1<< INT0 ) )
  #define TX_PIN           PD3               //!< Transmit data pin
  #define RX_PIN           PD1               //!< Receive data pin, must be INT0
  #define TCCR             TCCR0A            //!< Timer/Counter Control Register
  #define TCCR_P           TCCR0A            //!< Timer/Counter Control (Prescaler) Register
  #define OCR              OCR0A             //!< Output Compare Register
  #define EXT_IFR          EIFR              //!< External Interrupt Flag Register
  #define EXT_ICR          EICRA             //!< External Interrupt Control Register
  #define TIMER_COMP_VECT  TIMER0_COMP_vect  //!< Timer Compare Interrupt Vector

#elif (defined(__ATmega48__) || defined(__ATmega88__))
  #define ENABLE_TIMER_INTERRUPT( )       ( TIMSK0 |= ( 1<< OCIE0A ) )
  #define DISABLE_TIMER_INTERRUPT( )      ( TIMSK0 &= ~( 1<< OCIE0A ) )
  #define CLEAR_TIMER_INTERRUPT( )        ( TIFR0 |= ((1 << OCF0A) ) )
  #define ENABLE_EXTERNAL0_INTERRUPT( )   ( EIMSK |= ( 1<< INT0 ) )
  #define DISABLE_EXTERNAL0_INTERRUPT( )  ( EIMSK &= ~( 1<< INT0 ) )
  #define TX_PIN           PD3                //!< Transmit data pin
  #define RX_PIN           PD2                //!< Receive data pin, must be INT0
  #define TCCR             TCCR0A             //!< Timer/Counter Control Register
  #define TCCR_P           TCCR0B             //!< Timer/Counter Control (Prescaler) Register
  #define OCR              OCR0A              //!< Output Compare Register
  #define EXT_IFR          EIFR               //!< External Interrupt Flag Register
  #define EXT_ICR          EICRA              //!< External Interrupt Control Register
  #define TIMER_COMP_VECT  TIMER0_COMPA_vect  //!< Timer Compare Interrupt Vector

#else
  #warning Selected AVR device is not supported
#endif


#define TRXDDR  DDRD
#define TRXPORT PORTD
#define TRXPIN  PIND

#define SET_TX_PIN( )    ( TRXPORT |= ( 1 << TX_PIN ) )
#define CLEAR_TX_PIN( )  ( TRXPORT &= ~( 1 << TX_PIN ) )
#define GET_RX_PIN( )    ( TRXPIN & ( 1 << RX_PIN ) )


/*! \brief  Type defined enumeration holding software UART's state.
 *
 */
typedef enum
{
    IDLE,                                       //!< Idle state, both transmit and receive possible.
    TRANSMIT,                                   //!< Transmitting byte.
    TRANSMIT_STOP_BIT,                          //!< Transmitting stop bit.
    RECEIVE,                                    //!< Receiving byte.
    DATA_PENDING                                //!< Byte received and ready to read.

}AsynchronousStates_t;

static volatile AsynchronousStates_t state;     //!< Holds the state of the UART.
static volatile unsigned char SwUartTXBitCount; //!< TX bit counter.
static volatile unsigned char SwUartRXData;     //!< Storage for received bits.
static volatile unsigned char SwUartRXBitCount; //!< RX bit counter.


/*! \brief  External interrupt service routine.
 *
 *  The falling edge in the beginning of the start
 *  bit will trig this interrupt. The state will
 *  be changed to RECEIVE, and the timer interrupt
 *  will be set to trig one and a half bit period
 *  from the falling edge. At that instant the
 *  code should sample the first data bit.
 *
 *  \note  initSoftwareUart( void ) must be called in advance.
 */

ISR ( INT0_vect )
{

  state = RECEIVE;                  // Change state
  DISABLE_EXTERNAL0_INTERRUPT( );   // Disable interrupt during the data bits.

  DISABLE_TIMER_INTERRUPT( );       // Disable timer to change its registers.
  TCCR_P &= ~( 1 << CS01 );         // Reset prescaler counter.

  TCNT0 = INTERRUPT_EXEC_CYCL;      // Clear counter register. Include time to run interrupt rutine.

  TCCR_P |= ( 1 << CS01 );          // Start prescaler clock.

  OCR = TICKS2WAITONE_HALF;         // Count one and a half period into the future.

  SwUartRXBitCount = 0;             // Clear received bit counter.
  CLEAR_TIMER_INTERRUPT( );         // Clear interrupt bits
  ENABLE_TIMER_INTERRUPT( );        // Enable timer0 interrupt on again

}


/*! \brief  Timer0 interrupt service routine.
 *
 *  Timer0 will ensure that bits are written and
 *  read at the correct instants in time.
 *  The state variable will ensure context
 *  switching between transmit and recieve.
 *  If state should be something else, the
 *  variable is set to IDLE. IDLE is regarded
 *  as a safe state/mode.
 *
 *  \note  initSoftwareUart( void ) must be called in advance.
 */
SRI ( TIMER_COMP_VECT )
{

  switch (state) {
  // Transmit Byte.
  case TRANSMIT:
    // Output the TX buffer.
    if( SwUartTXBitCount < 8 ) {
      if( SwUartTXData & 0x01 ) {           // If the LSB of the TX buffer is 1:
        SET_TX_PIN();                       // Send a logic 1 on the TX_PIN.
      }
      else {                                // Otherwise:
        CLEAR_TX_PIN();                     // Send a logic 0 on the TX_PIN.
      }
      SwUartTXData = SwUartTXData >> 1;     // Bitshift the TX buffer and
      SwUartTXBitCount++;                   // increment TX bit counter.
    }

    //Send stop bit.
    else {
      SET_TX_PIN();                         // Output a logic 1.
      state = TRANSMIT_STOP_BIT;
    }
  break;

  // Go to idle after stop bit was sent.
  case TRANSMIT_STOP_BIT:
    DISABLE_TIMER_INTERRUPT( );           // Stop the timer interrupts.
    state = IDLE;                         // Go back to idle.
    ENABLE_EXTERNAL0_INTERRUPT( );        // Enable reception again.
  break;

  //Receive Byte.
  case RECEIVE:
    OCR = TICKS2WAITONE;                  // Count one period after the falling edge is trigged.
    //Receiving, LSB first.
    if( SwUartRXBitCount < 8 ) {
        SwUartRXBitCount++;
        SwUartRXData = (SwUartRXData>>1);   // Shift due to receiving LSB first.
        if( GET_RX_PIN( ) != 0 ) {
            SwUartRXData |= 0x80;           // If a logical 1 is read, let the data mirror this.
        }
    }

    //Done receiving
    else {
        state = DATA_PENDING;               // Enter DATA_PENDING when one byte is received.
        DISABLE_TIMER_INTERRUPT( );         // Disable this interrupt.
        EXT_IFR |= (1 << INTF0 );           // Reset flag not to enter the ISR one extra time.
        ENABLE_EXTERNAL0_INTERRUPT( );      // Enable interrupt to receive more bytes.
    }
  break;

  // Unknown state.
  default:
    state = IDLE;                           // Error, should not occur. Going to a safe state.
  }

  return 0;
}


/*! \brief  Function to initialize the software UART.
 *
 *  This function will set up pins to transmit and
 *  receive on. Control of Timer0 and External interrupt 0.
 *
 *  \param  void
 *
 *  \retval void
 */
static void SUART_init( void )
{
  //PORT
  TRXPORT |= ( 1 << RX_PIN );       // RX_PIN is input, tri-stated.
  TRXDDR |= ( 1 << TX_PIN );        // TX_PIN is output.
  SET_TX_PIN( );                    // Set the TX line to idle state.

  // Timer0
  DISABLE_TIMER_INTERRUPT( );
  TCCR = 0x00;    //Init.
  TCCR_P = 0x00;    //Init.
  TCCR |= (1 << WGM21);			// Timer in CTC mode.
  TCCR_P |=  ( 1 << CS21 );        // Divide by 8 prescaler.

  //External interrupt
  EXT_ICR = 0x00;                   // Init.
  EXT_ICR |= ( 1 << ISC01 );        // Interrupt sense control: falling edge.
  ENABLE_EXTERNAL0_INTERRUPT( );    // Turn external interrupt on.

  //Internal State Variable
  state = IDLE;
}


/*! \brief  Send a unsigned char.
 *
 *  This function sends a unsigned char on the TX_PIN
 *  using the timer0 isr.
 *
 *  \note  initSoftwareUart( void ) must be called in advance.
 *
 *  \param  c    unsigned char to transmit.
 *
 *  \retval void
 */
void putChar( const unsigned char c )
{
  while( state != IDLE )
  {
    ;                               // Don't send while busy receiving or transmitting.
  }

  state = TRANSMIT;
  DISABLE_EXTERNAL0_INTERRUPT( );  // Disable reception.
  SwUartTXData = c;             // Put byte into TX buffer.
  SwUartTXBitCount = 0;

  TCCR_P &= ~( 1 << CS01 );         // Reset prescaler counter.
  TCNT0 = 0;                        // Clear counter register.
  TCCR_P |= ( 1 << CS01 );          // CTC mode. Start prescaler clock.

  CLEAR_TX_PIN();               // Clear TX line...start of preamble.

  CLEAR_TX_PIN( );                   // Clear TX line...start of preamble

  ENABLE_TIMER_INTERRUPT( );        // Enable interrupt
}


/*! \brief  Print unsigned char string.
 *
 *  \param  const unsigned char*    Pointer to the string that is to be printed.
 *
 *  \retval void
 */
void suart_print( const unsigned char *data )
{
  while( *data != '\0' )
  {
    putChar( *data++ );
  }
}


/*! \brief  Main loop.
 *
 *  This loop will run forever.
 *
 *  \note  This main method will write back a predefined string depending upon the character received from the user.
 *  \note  Only send one character at the time, or the loop will hang. To enable receiving of multiple chars look at
           AVR306: Using the AVR UART. This application note describes a buffered solution.
 *  \param  void
 *
 *  \retval void
 */
/**
int main( void )
{
  initSoftwareUart( );
  sei();

  for( ; ; )
  {
    if( state == DATA_PENDING )
    {
      state = IDLE;

      switch( SwUartRXData )
      {

        case 'a':
            print_string( "atmel avr\r\n" );
        break;

        case 'A':
            print_string( "Atmel AVR\r\n" );
        break;

        default:
            print_string( "Unknown command\r\n" );
        break;
      }
    }
  }
}
*/
