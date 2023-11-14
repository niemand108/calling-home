Feel free to add resistors and capacitors in some places
```
                                                          ┌──────►  INT arduino
                                                          │
                                                          │
                                                5V arduino│
                                                    │     │
                                                    │     │   ┌────────┐      │
                                          ──────┐   │    ┌┴───┤   R    ├──────┤ ┼   GND Arduino
                                                │   │    │    └────────┘      │
                                                │   │    │
                                                │   │    │
                                              ┌─┴───┴────┴─────┐
                                              │    4N25        │
                                              ├┐               │
                                              ├┘octocoupler    │
                                              │                │
     SPEAKER OUT(+)                           └─┬───┬───┬──────┘
       │                         │              │   │   │
       │                         │      external Vcc│
       │                      NPN├──────────────────┘
     ┌─┴─────────────────────────┤
     │                      2n222├──────┐
     │                           │      │
     │                           │      ▼
  SPEAKER IN(+)                         │
                                        │
                                        ├────── SPEAKER IN (-)
                                        │
                                        ├────── SPEAKER OUT (-)
                                        │
                                        │
                                        │
                                       ─┴──
                                        external GND
                                        
                                        
```
