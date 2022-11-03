#pragma once

//! Lock unlock keyboard status
typedef enum kbd_lock_status {
    kbd_unlocked,       //! kbd_ lock
    kbd_locked      //! kbd_ unlock
} kbd_lock_status_t;

//! Key indentifiers
typedef enum kbd_key {
    key_kbd_none = 0,    //! No any events in the kbd queue
    kbd_key_numeric1 = 31,
    kbd_key_numeric2 = 32,
    kbd_key_numeric3 = 33,
    kbd_key_numeric4 = 41,
    kbd_key_numeric5 = 42,
    kbd_key_numeric6 = 43,
    kbd_key_numeric7 = 51,
    kbd_key_numeric8 = 52,
    kbd_key_numeric9 = 53,
    kbd_key_numeric0 = 62,
    kbd_key_numeric_ast = 61,
    kbd_key_numeric_pnd = 63,

    kbd_joy_left = 11,
    kbd_joy_right = 13,
    kbd_joy_up = 2,
    kbd_joy_down = 22,
    kbd_joy_enter = 12,

    kbd_fn_left = 21,
    kbd_fn_right = 23,

    kbd_vol_up = 4,
    kbd_vol_down = 14,
    kbd_torch = 24,

    kbd_sswitch_up = 34,
    kbd_sswitch_down = 54,
    kbd_sswitch_mid = 44

} kbd_key_t;


//! kbd_ event type 
typedef enum kbd_evtype {
    kbd_pressed,    //! KBD is presssed
    kbd_released    //! KBD is released

} kbd_evtype_t;

//! kbd_ event status
typedef struct kbd_event {
    kbd_evtype_t event;     //! Event status
    kbd_key_t key;          //! Key pressed
} kbd_event_t;

/** Initialize the keyboard engine */
int kbd_init();


/** Lock or unlock the keyboard
 * @param lock Lock or unlock status
 * @return Zero on sucess otherwise error code
 */
int kbd_lock(kbd_lock_status_t lock);

/** Read the key events
 * @return table containing events on success otherwise NULL
 */
kbd_event_t *kbd_read_events();
