#![no_std]
#![no_main]

use heapless::Vec;
use panic_halt as _;

use embedded_hal::serial::Read;
use arduino_hal::prelude::*;
use arduino_hal::port::Pin;
use core::cell::Cell;


struct IOModuleDirect {
   pin: Cell<Pin>,
}


//
// Traits
//

// Have a component set itself up
trait Initialize {
    fn initialize(&self);
}

// Poll a component to see it's current state
trait Poll {
    fn poll(&self) -> bool;
}

// Get a message for the current component
trait GetMessage {
    fn getMessage(&self, msg: &mut str);
}

trait SetComponent {
    fn set(state: bool);
}

//
// Structs
//
#[derive(Debug)]
struct Switch {
    id: &'static str,
    state: bool,
}

#[derive(Debug)]
struct Button {
    id: &'static str,
    state: bool,
}

#[derive(Debug)]
enum LedState {
    Off,
    On,
    FlashingOn,
    FlashingOff,
}

#[derive(Debug)]
struct Led {
    id: &'static str,
    state: LedState,
}

#[derive(Debug)]
struct SevenSeg {
    id: &'static str,
    value: u8,
}


enum InputComponent {
    Switch(Switch),
    Button(Button),
}

enum OutputComponent {
    Led(Led),
    SevenSeg(SevenSeg),
}

#[arduino_hal::entry]
fn main() -> ! {
    let dp = arduino_hal::Peripherals::take().unwrap();
    let pins = arduino_hal::pins!(dp);
    let foo: bool = pins.d13;
    let mut led: u32 = pins.d13.into_output();

    let mut components: Vec<OutputComponent, 3> = Vec::new();

    let mut serial = arduino_hal::default_serial!(dp, pins, 115200);
    ufmt::uwriteln!(&mut serial, "Hello from Arduino!\r").void_unwrap();

    // Read a byte from the serial connection
    let b = nb::block!(serial.read()).void_unwrap();

    // for i in 0..9 {
    //    buf[i] = block!(uart_rx.read())?;
    //}

//     components.push( 
//         InputComponent::Switch(Switch { 
//             id: "SW1", 
//             state: false,
//         }) );

//     components.push( 
//         InputComponent::Button(Button { 
//             id: "BT1", 
//             state: false,
//         }) );

    components.push( 
        OutputComponent::Led(Led { 
            id: "Led", 
            state: LedState::Off,
        }) );

    /*
     * For examples (and inspiration), head to
     *
     *     https://github.com/Rahix/avr-hal/tree/main/examples
     *
     * NOTE: Not all examples were ported to all boards!  There is a good chance though, that code
     * for a different board can be adapted for yours.  The Arduino Uno currently has the most
     * examples available.
     */


    loop {
        led.toggle();
        arduino_hal::delay_ms(2000);
    }
}
