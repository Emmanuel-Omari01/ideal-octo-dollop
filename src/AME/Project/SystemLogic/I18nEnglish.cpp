/*
 * I18nEnglish.cpp
 *
 *  Created on: April 29, 2024
 *      Author: D. Schulz, AME
 *  Copyright (c) 2024 Andreas Müller electronic GmbH (AME)
 */

typedef const char * const string;
    static string const i18nEnglish[600] = {
        "PLC commands: ",                                                       // 00
        "plcInit",                                                              // 01
        "plcJoin",                                                              // 02
        "plcForceJoin",                                                         // 03
        "forceNetMember",                                                       // 04
        "searchNetwork",                                                        // 05
        "getADPAttribute",                                                      // 06
        "setADPAttribute",                                                      // 07
        "getEAPAttribute",                                                      // 08
        "setEAPAttribute",                                                      // 09
        "getMacAttribute",                                                     // 10
        "setMacAttribute",                                                     // 11
        "setClientInfo",                                                       // 12
        "showMembers",                                                         // 13
        "sendMessage",                                                         // 14
        "setVerbose",                                                          // 15
        " ",                                                                   // 16
        "pathDiscovery",                                                       // 17
        "remotePathDisc",                                                      // 18
        "peerRouting",                                                         // 19
        "coordRouting",                                                        // 20
        "startAliveCheck",                                                     // 21
        "AliveInterval",                                                       // 22
        "AliveNextPeer",                                                       // 23
        "---------------------------------------------------------------",     // 24
        "[P|C] Initialize the modem as peer or coordinator",                   // 25
        "[panID] [lbAddress] joins peer to a PLC network",                     // 26
        "[netId] [OwnAddress] forces a join process",                          // 27
        "[ListIndex][IpAdr][shortAddress][link] link:"
            " 0=disconnected, 1=pending, 2=rejected, 3=connected, 4=not verified",  // 28
        "[seconds] search for x seconds for a network",                        // 29
        "[identifier][table index]",                                           // 30
        "[identifier][table index][attribute]",                                // 31
        "[validFlag][extendedAddress][networkAddress]",                        // 32
        "shows all network participants",                                      // 33
        "[N/B] [memberID] [message] sends a message to a member B=broadcast",  // 34
        "[mask] set bit mask to change the terminal outputs: 1FFE=everything, 0=nothing",  // 35
        " ",                                                                    // 36
        "[memberID][metric](default: 15) performs a path discovery to a member",  // 37
        "[sourceID][destID] performs a path discovery between 2 members",      // 38
        "starts the routing between the peers",                                // 39
        "starts the routing between coordinator and all peers",                // 40
        "[0|1] 1=starts the alive control at the coordinator",                 // 41
        "[DelayMs] sets the alive interval at the coordinator",                // 42
        "[NextID] [PrevID] determine the predecessor and successor for the alive",  // 43
        "Motor commands:",                                                      // 44
        "toggleRelay",                                                          // 45
        "getMultiLevel",                                                        // 46
        "getAnalogLevel",                                                       // 47
        "resetSystem",                                                          // 48
        "Commands for the data flash:",                                         // 49
        "eraseDataFlash",                                                       // 50
        "setID",                                                                // 51
        "getID",                                                                // 52
        "memoryWriteTest",                                                      // 53
        "memoryReadTest",                                                       // 54
        "exitSearch",                                                           // 55
        "[Index]     sets an output with the specified index.",                 // 56
        "[Index]     returns the voltage level of the specified multiplexer channel.",  // 57
        "[Index]     returns the voltage level of the specified analog pin.",   // 58
        "restarts the devices.",                                                // 59
        "[block]   deletes an block within the data flash. Default:0",          // 60
        "[key] [data] writes data to the data flash",                           // 61
        "[key]        reads data from the data flash",                          // 62
        "==> pre-initializing of the channel is completed",                     // 63
        "==> the frequency band of the channel is set",                         // 64
        "=> ADP Layer is successful restarted",                                 // 65
        "=> EAP Layer is successful restarted",                                 // 66
        "=> EAP Layer refusal of client information is successful",             // 67
        "=> EAP Layer repeat option in EAP error state is successful",          // 68
        "=> ADP Layer setting of the type of the device is successful",         // 69
        "=> the default route of the coordinator is set",                       // 70 (obsolete?)
        "Initializing of the channel is completed.",                            // 71
        "The network is going to be initialized.",                              // 72
        "=> Group Master Key was set successful",                               // 73
        "=> Network started",                                                   // 74
        "=> EAP authentication process started!",                               // 75
        "Network initialized.",                                                 // 76
        "Firmware transmission successful.",                                    // 77
        "\nA new device is going to be registered.",                            // 78
        "Transmission successful.",                                             // 79
        "Setting of device information in the modem is successful.",            // 80
        "Join process is successful.",                                          // 81
        "The modem has left the network.",                                      // 82
        "The deletion of the member is successful",                             // 83
        "The routing attempt is started.",                                      // 84
        "The route could not be determined.",                                   // 85
        "The route to the target could not be found.",                          // 86
        "Reading of the mac frame counters is successful.",                     // 87
        "Loading of the mac frame counters is successful.",                     // 88
        "The firmware could not be transmitted to the modem.",                  // 89
        "Network scanning completed.",                                          // 90
        "ADP attribute adjusted.",                                              // 91
        "EAP attribute adjusted.",                                              // 92
        "Message receive token: %d",                                            // 93
        "/n/nLegend: c := cost in SNR // [xxx] := PLC-knot // --> := connection between 2 knots",  // 94
        "/n-------------------------------Path discovery finished----------------------------------",    // 95
        "The message could not be sent due to inactivity.",                     // 96
        "ID was not recognized.",                                               // 97
        "Start Routing on PAN-ID: %04X ",                                       // 98
        "-------------------------------Start path discovery on PAN-ID: %04X ----------------------",    // 99
        "Remote path discovery is triggered.",                                 // 100
        "EAP attribute requested.",                                            // 101
        "MAC attribute requested.",                                            // 102
        "ADP attribute requested.",                                            // 103
        "MAC attribute is set.",                                               // 104
        "ADP attribute is set.",                                               // 105
        "EAP attribute is set.",                                               // 106
        "Attribute will be set.",                                              // 107
        "Incorrect input. Only 'P' or 'C' is accepted.",                       // 108
        "G3 channel is going to be initialized.",                              // 109
        "Verbose mode adjusted.",                                              // 110
        "Join process triggered.",                                             // 111
        "Force Join triggered.",                                               // 112
        "The routes to the members will be adjusted.",                         // 113
        "Alive checking is a coordinator command only.",                       // 114
        "The Message [ %s ] is send to member with IP address: %d",            // 115
        "Entry will be registered",                                            // 116
        "Invalid arguments",                                                   // 117
        "\rThe search starts.",                                                // 118
        "Alive checking is %s released.",                                      // 119
        "not",                                                                 // 120
        "%s value %d set.",                                                    // 121
        "AutostartFlow",                                                       // 122
        "SetTargetNetID",                                                      // 123
        "SetOwnNetworkAddress",                                                // 124
        "[mode] 0:network search, 1=network force join, 3=force join with verification",  // 125
        "[id]  0...65535 ID of the desired network",                           // 126
        "[adr] 0...65535 address of the own network",                          // 127
        "\nThe auto start process could be interrupted with the \"exitSearch\" command.",  // 128
        "plcInit C",                                                           // 129
        "plcInit P",                                                           // 130
        "Restarting of boot loader.",                                          // 131
        "Restarting of the system.",                                           // 132
        "Restarting initialization",                                           // 133
        "Meantime: %s",                                                        // 134
        "Network scan timeout.",                                               // 135
        "Start joining on PAN-ID: %04X and LBA-agent: %04X",                   // 136
        "Start force joining on PAN-ID: %04X with network address: %04X",      // 137
        "Join failed.",                                                        // 138
        "Waiting for the next try",                                            // 139
        "Restarts join.",                                                      // 140
        "\rPLC autostart completed.",                                          // 141
        "Start-time: %s",                                                      // 142
        "Deleting of the last used network agent.",                            // 143
        "\rForceJoin check transmission",                                      // 144
        "\rForceJoin check acknowledge",                                       // 145
        "Starts routing to member: %d",                                        // 146
        "MemberRouting completed.",                                            // 147
        "Routing interrupted.",                                                // 148
        "MemberRouting wait timeout",                                          // 149
        " ",                                                                   // 150
        " ",                                                                   // 151
        " ",                                                                   // 152
        " ",                                                                   // 153
        " ",                                                                   // 154
        "An alive checking is not provided for this member.",                  // 155   obsolete?
        "Alive-interval: %d ms",                                               // 156
        "Incoming messages on channel 1 are not provided.",                    // 157
        "\nMetric: %d",                                                        // 158
        "[%03d]",                                                              // 159
        "--{c %+03.2f dB}-->",                                                 // 160
        "Modem answer",                                                        // 161
        "\nModem request",                                                     // 162
        "Modem software will be transmitted:",                                 // 163
        "- Segment %c was requested.",                                         // 164
        "- Changing of baud rate was requested.",                              // 165
        "- Baud rate 115200bps is accepted.",                                  // 166
        "Message:",                                                            // 167
        "Signal quality: %03.2f dB (SNR)",                                     // 168
        "Erroneous switch matrix command",                                     // 169
        "Transmit K to member: %d",                                            // 170
        "-->> %s",                                                             // 171
        "End of member list.",                                                 // 172
        "Network member with IP %d was deactivated.",                          // 173
        "No network members available.",                                       // 174
        "List of network members:",                                            // 175
        "%03d is coordinator (1=yes): %d",                                     // 176
        "PLC board number:",                                                   // 177
        "No channel with the specified index available.",                      // 178
        "Channel has HIGH level.",                                             // 179
        "Channel has LOW level.",                                              // 180
        "sendMessage 0 S%d",                                                   // 181
        "Analog pin %d: %f V",                                                 // 182
        "ID: %d is set.",                                                      // 183
        "Flash block was deleted.",                                            // 184
        "\nDiscovery container results:\n",                                    // 185
        "Network ID: %04X  LBA agent: %04X link quality: %04X RoutingCost: %04X\n",  // 186
        "Alive MonitorLog: current[%4d]ms | median [%4.2f]ms | max [%4d]ms | min [%4d]ms",  // 187
        "Auto start interrupted.",                                             // 188
        "Member does not react on transmission:",                              // 189
        "Alive erroneous transmission. Transmission repeated:",                // 190
        "The following member is joined to the network:",                      // 191
        "The following member is rejoined to the network:",                    // 192
        // Coordinator routing
        "%sRC%03d#%03d",                                                       // 193
        "Routing: coordinator--->peer[%03d]",                                  // 194
        "----------------------routing completed---------------------------",  // 195
        "getPLCAttributes",                                                    // 196
        "Request of PLC attributes",                                           // 197
        "Request of attributes is started.",                                   // 198
        "Force join parameters of coordinator:",                               // 199
        "setFJCallOuts",                                                       // 200
        "[n] Number of repetition tries",                                      // 201
        "Callouts are set to %d.",                                             // 202
        "setFJDelay",                                                          // 203
        "[X|Y|Z] [n] Dwell in seconds",                                        // 204
        "Dwell %c is set to %d seconds.",                                      // 205
        "setFJScale",                                                          // 206
        "Scaling factor for dwell",                                            // 207
        "Scaling factor is set to %d.",                                        // 208
        "RNetworkSearch",                                                      // 209
        "remote search network [[Peer n] [SearchTime t]] or 'S'=Search with all members.",  // 210
        "RNetworkShow",                                                        // 211
        "[n] show discovery information optionally for peer n",                // 212
        "Remote discovery peer entries",                                       // 213
        "No more entries",                                                     // 214
        "Peer IP:%d",                                                          // 215
        "Coordinator",                                                         // 216
        "\rDescriptor IP LQI",                                                 // 217
        "    %%",                                                              // 218
        "   %d       %d   %d",                                                 // 219
        "%u %u",                                                               // 220
        "\rRemote discovery finished.",                                        // 221
        "%s%cR%03d %02d",                                                      // 222
        "\rDiscovery @peer %04d",                                              // 223
        "\rDiscovery @coordinator",                                            // 224
        "IP number too high",                                                  // 225
        "Member not active",                                                   // 226
        "Searching time invalid.",                                             // 227
        "setTestSuite",                                                        // 228
        "[enable] [p] run or stop test suite (run: enable<>0). p=number of test",  // 229
        "getSystemVersion",                                                    // 230
        "retrieve PLC system version",                                         // 231
        "%s value %d, %d set.",                                                // 232
        "Peer",                                                                // 233
        "last_test",                                                           // 234
        "[last_test] [true|false] specify if last unit or integration test is running. default=true",  // 235
        "Hardware version: %s",                                                // 236
        "unknown",                                                             // 237
        " ",                                                                   // 238
        "success",                                                             // 239
        "failed ",                                                             // 240
        " ",                                                                   // 241 txtEmpty
        "true",                                                                // 242
        "false",                                                               // 243
        "HIGH",                                                                // 244
        "LOW ",                                                                // 245
        "not",                                                                 // 246
        "none",                                                                // 247
        "yes",                                                                 // 248
        "no",                                                                  // 249
        // Integration tests
        "Integration test: Number not available",                              // 250
        "Number of tested round trips: %s",                                    // 251
        "The automation of the alive was started with a minimum number of %s peers.",  // 252
        "Actually registered peers: %s",                                       // 253
        "Minimum spanning tree created: %s",                                   // 254
        "Alive route is registered within the peers: %s",                      // 255
        "Peers who are not participants of the alive round trips any more:",   // 256
        "Alive round trip successfully completed.",                            // 257
        "Peer %d is not participant of the alive round trips after %d seconds",  // 258
        "Peer %d is participant to the alive round trips again after %d seconds",  // 259
        "Test is running for %d seconds",                                      // 260
        "Number of round trips with repeated timeouts: %s",                    // 261
        "Peer who temporarily did not participate to the alive round trips:",  // 262
        " ",                                                                   // 263
        " ",                                                                   // 264
        " ",                                                                   // 265
        " ",                                                                   // 266
        " ",                                                                   // 267
        " ",                                                                   // 268
        " ",                                                                   // 269
        // automatic, AssignmentFlow
        "\r%d groups are set at coordinator",                                  // 270
        "\r%d groups are set at IP %d",                                        // 271
        "\rThe following peer are in the 'pending' state and are not included:",  // 272
        "\rProcess aborted.",                                                  // 273
        "Timeout outer control is created.",                                   // 274
        "Refresh timeout outer control",                                       // 275
        "Timeout outer control",                                               // 276
        "\r%u broadcast groups are restored.",                                 // 277
        "Alive automatic, %.30s main step=%u, sub step=%u, with automatic=%c",  // 278
        "All timeouts within the automatic are deleted.",                      // 279
        "\rA delay of 5 minutes is triggered again.",                          // 280
        "\rOuter timeout within the automatic restarted.",                     // 281
        "\rTimeouts of the route discovery are deleted.",                      // 282
        "During the route discovery only %d peers are notified. Process aborted.",  // 283
        "\rOuter timeout deleted.",                                            // 284
        "\rInner timeouts are stopped within the automatic.",                  // 285
        "\rTimeout for route discovery is created.",                           // 286
        "Timeout of route discovery occurred.",                                // 287
        "Callout of route discovery occurred.",                                // 288
        "Adjacency matrix is going to be filled.",                             // 289
        "shortened tree:",                                                     // 290
        "broadcast groups are going to be distributed.",                       // 291
        "The minimum number of peers are not reached after the distribution.",  // 292
        "\rRequested:%u, connected:%u",                                        // 293
        "Repetition of creating the spanning tree.",                           // 294
        "Starting of the alive round trips are now possible.",                 // 295
        "Terminal outputs are reduced now.",                                   // 296
        "\rAlive round trips are going to be started.",                        // 297
        "\r%d minutes dwell time remaining.",                                  // 298
        "Boot system start.",                                                  // 399
        "Ignore force join checks of %s for %u times",                         // 300
        "Repeated treatment of the alive acknowledge suppressed.",             // 301
        "Function transmitAliveText, error group 0",                           // 302
        "Group address %03d is fallen out: erroneous state classification",    // 303
        "round trip control: timeout %03d of %03d",                            // 304
        "\rH,h,?           Show this help screen, consider upper and lower case",  // 305
        "Member rejected.",                                                    // 306
        "Untreated join state %02X",                                           // 307
        "Member does not response (timeout):",                                 // 308
        "Member with IP %03d does not response",                               // 309
        "Continue",                                                            // 310
        "Deactivation of network member %d is rejected",                       // 311
        "Reporter is last member in the alive route",                          // 312
        "There is already another attempt for an alive route loss in progress",  // 313
        "Request was handled with the same IP and round trip number",          // 314
        "Error in JumpMapper::informUserAboutInvalidity",                      // 315
        "Alive jump request acknowledged",                                     // 316
        "Start alive round trip with IP %03d",                                 // 317
        "Request ignored. Round trip already in request state",                // 318
        "Member IP %03d reports an loss of IP %03d!",                          // 319
        "Error: jump request message received, timeout ID %03d already started",  // 320
        "Timeout with ID %03d was not initialized",                            // 321
        "Number of vertices: %d",                                              // 322
        "Weight of vertex (%u,%u): %d",                                        // 323
        "Origin of matrix: %p",                                                // 324
        "Route %d -> %d %c",                                                   // 325
        "The maximum route length has been reached.",                          // 326
        "The spanning tree is cleaved:",                                       // 327
        "#%d Route %u -> %u. Depth:%u",                                        // 328
        "Adjacency matrix",                                                    // 329
        "Acknowledge received",                                                // 330
        "Remote discovery transmission ignored once",                          // 331
        "Remote discovery request ignored once",                               // 332
        "%u Remote discovery requests will be ignored",                        // 333
        "Remote discovery data transmission to coordinator is suppressed %u times",  // 334
        "Remote discovery: Search is started.",                                // 335
        "simpleDiscovery::nextAutoDiscovery. Pending mask: %x, index: %d",     // 336
        "simpleDiscovery::nextAutoDiscovery. Repeat once",                     // 337
        "\rsimpleDiscovery: Route discovery will be repeated with individual peers.",  // 338
        "\rsimpleDiscovery: Route discovery timeout cleared.",                 // 339
        "\rsimpleDiscovery: Route discovery timeout restarted.",               // 340
        "Timeout acknowledge",                                                 // 341
        "Timeout remote data",                                                 // 342
        "  Progress %d/%d",                                                    // 343
        "The alive round trips are bound to a superior test.\r\n"              // 344
        "Please apply command 'stop alive' first.",
        "Alive round trips are started.",                                      // 345
        "Alive round trips are stopped.",                                      // 346
        "The command is reserved in coordinator mode only",                    // 347
        "The command is reserved in peer mode only",                           // 348
        "globalReset",                                                         // 349
        "Restarts globally all device within the network",                     // 350
        "setDefaultParameter",                                                 // 351
        "set default parameters",                                              // 352
        "getAliveMapping",                                                     // 353
        "shows a reference table for the alive routing",                       // 354
        "editTimeout",                                                         // 355
        "[ID] [Delay] test: set delay of an internal timer (TimeoutDelayID = 2)",  // 356
        "incorrect entry",                                                     // 357
        "The timeout delay with ID %d was set to %d [50ms].",                  // 358
        "Global reset is executed.",                                           // 359
        "Row data are send to the PLC modem.",                                 // 360
        "== End of table ==",                                                  // 361
        "ID: %d",                                                              // 362
        "Store join information",                                              // 363
        "Random-delay:%ds",                                                    // 364
        "Force join timeout",                                                  // 365
        "Software reset",                                                      // 366
        "Initialization successful",                                           // 367
        "Start network search",                                                // 368
        "Force join step: %d",                                                 // 369
        "Force join state: %x",                                                // 370
        "Wait %d seconds for alive round trips",                               // 371
        "Verification requests J:%d",                                          // 372
        "plcleave",                                                            // 373
        "This command causes the modem to leave the network",                  // 374
        "sendPLCRaw",                                                          // 375
        "a b c ... z send raw data (hex. z.B. a=7E,b=00,c=...,z=7E) to the modem",  // 376
        "triggerAutomatic",                                                    // 377
        "[n] execute steps up to the alive test. n=min. number of peers (default=1)",  // 378
        "start",                                                               // 379
        "stop",                                                                // 380
        "alive [round trips] @coordinator: executes alive round trips within the network; -1=infinite",  // 381
        "integration test [test-#] [period] [minPeers] @coordinator: executes an integration test",  // 382
        "alive @coordinator: stops the alive round trips. It also stops the test case for the alive",  // 383
        "integration test @coordinator: stops the actual integration test",    // 384
        "autoSubSteps",                                                        // 385
        "DEBUGGING: [s1] [s2] starts steps for the alive test automatic. s1=main step, s2=sub step",  // 388
        "BlockStoreTable",                                                     // 387
        "[0|1] Expert: 1=suppress writing of the cycling routing information",  // 388
        "RBroadGroupSet",                                                      // 389
        "[target IP|all][offset][numTupel][memberID,BGIP] [..]",               // 390
        "ignoreFCRequests",                                                    // 391
        "[n] Expert: ignore force join checks at peer or coordinator for n times",  // 392
        "testMalloc",                                                          // 393
        "[n] Expert: heap test with n Bytes",                                  // 394
        "FillAdjMatrix",                                                       // 395
        "[m] Expert: Fill adjacency matrix with discovery results\r\n"         // 396
        "               and show spanning tree. m=minimum spanning tree",
        "showAdjMatrix",                                                       // 397
        "Show adjacency matrix",                                               // 398
        "presetAliveRoute",                                                    // 499
        "[IP1] [IP2] ... [IPn] Expert: preset route for the alive test",       // 400
        "registerVirtualPeers",                                                // 401
        "[maxIP] Expert: register virtual peers with maxIP as last IP-Address to register",  // 402
        "clearDeviceFC",                                                       // 403
        "[tableOffset][srcAddress] Expert: deletes the MacFrameCounter entry in the device table",  // 404
        "enableAliveRound",                                                    // 405
        "[Enable] Expert: Enables the round trip test for an peer; 1=Enable, 0=block",  // 406
        "poll",                                                                // 407
        "[target IP] sends an poll command to the target IP and switches the outputs alternating",  // 408
        "input",                                                               // 409
        "[target IP] <input-nr.> Shows the requested input. With no input no all inputs are shown.",  // 410
        "output",                                                              // 411
        "[target IP] <relay> <level> sets output <relay> {0,1,2,3} to level <level> {0,1}",  // 412
        "sendGlobalTimeout",                                                   // 413
        "[id] [delay] [timeouts] [callouts] Expert: send parameters for timeout settings to all peers",  // 414
        "i2cRead",                                                             // 415
        "Expert: Read sample data from i2c chip",                              // 416
        "i2cWrite",                                                            // 417
        "Expert: Write sample data to i2c chip",                               // 418
        "i2cInit",                                                             // 419
        "Expert: Initialize i2c bus driver",                                   // 410
        "i2cRd Mac",                                                           // 421
        "Read mac address from i2c chip",                                      // 422
        "Invalid number of arguments",                                         // 423
        "Leave network command triggered",                                     // 424
        "Terminal",                                                            // 425
        "Loops: %u",                                                           // 426
        "Alive test is %s enabled. %s",                                        // 427
        "Test restore of join tables",                                         // 428
        "Test function #%d",                                                   // 429
        "LineEntryTest",                                                       // 420
        "[n] Expert: test function #n",                                        // 431
        "Argument is stored.",                                                 // 432
        "block store table state is %d",                                       // 433
        "malloc fails with return value:%p",                                   // 434
        "%d Bytes are available in the heap memory.",                          // 435
        "virtual peers are registered up to IP %u",                            // 436
        "verify",                                                              // 437
        "coordinator may not requested.",                                      // 438
        "Global timeouts are going to be send.",                               // 439
        "alive",                                                               // 440
        "integration test",                                                    // 441
        "integration test already stopped.",                                   // 442
        "Reason of the last reset: %s",                                        // 443
        "\nModem request",                                                     // 444
        "Modem answer",                                                        // 445
        "Searching for frame counter entry",                                   // 446
        "Failure status code: %s",                                             // 447
        "source IP: %03d",                                                     // 448
        "Loss of member successful reported",                                  // 449
        "Timeout values set",                                                  // 450
        "external timeout values rejected",                                    // 451
        "force join check is ignored once",                                    // 452
        "Member=Null: %d Mac-Ptr %x IP-Ptr %x !Disconnected: %d",              // 453
        "force join check received. IP:%04d",                                  // 454
        "Addresses are going to be assigned",                                  // 455
        "Addresses are ignored once",                                          // 456
        "interpretation error",                                                // 457
        "Group assignment successful!",                                        // 458
        "Handle external reset request",                                       // 459
        "frame counter deletion finished",                                     // 460
        "no frame counter entry found",                                        // 461
        "RoundTrip #%d %s",                                                    // 462
        "finished",                                                            // 463
        "in progress",                                                         // 464
        "Alive round trips are finished!",                                     // 465
        "processAssignment",                                                   // 466
        "incFlowStep",                                                         // 467
        "triggerShowNetworkDiscoveryResults",                                  // 468
        "remoteBroadcastGroupSelection",                                       // 469
        "doCoordRouting",                                                      // 470
        "coordRoutingCallBack",                                                // 471
        "doRouteDiscovery",                                                    // 472
        "doBuildTree",                                                         // 473
        "doProofTree",                                                         // 474
        "doAssignAutomatic",                                                   // 475
        "assignmentCallOut",                                                   // 476
        "doStartAlive",                                                        // 477
        "user interface",                                                      // 478
        "timeoutRoutingStart",                                                 // 479
        "Only %d peers are available. Please correct the input",               // 480
        "Please enter the correct number of arguments",                        // 481
        "Tuple input error",                                                   // 482
        "No spanning tree or routing information available",                   // 483
        "The assignment of the broadcast groups is %d times ignored",          // 484
        "Group will be set: previous G: %d next G: %d acknowledge G: %d",      // 485
        "peerIP %d receiverGroup %d transmitterGroup %d ||",                   // 486
        "Minimum spanning tree cleaved. Neighbour search is restarted.",       // 487
        "Minimum spanning tree cleaved. Automatic stopped.",                   // 488
        "Minimum spanning tree is all right.",                                 // 489
        "The alive route is set by terminal.",                                 // 490
        "{Role: %s IP: %03d}",                                                 // 491
        "timeOutCounter: %d",                                                  // 492
        "\r5min automatic delay is started.",                                  // 493
        "Minimum amount of peers is not reached.\rRequired:%u, connected:%u",  // 494
        "Assignment timeout!",                                                 // 495
        "Polling: device poll state is %s",                                    // 496
        "active",                                                              // 497
        "stopped",                                                             // 498
        "Error: (sendIoCmdRequest): wrong command input!",                     // 599
        "%s request was send to member with IP %03d.",                         // 500
        "Wrong pin state reply type!",                                         // 501
        "Request was replied.",                                                // 502
        "Output: output with index %03d is illegal!",                          // 503
        "Output: output with index %03d was set to %s.",                       // 504
        "The request was not replied even though it was repeated!",            // 505
        "Input %d.%d: %u\n",                                                   // 506
        "Output %d.%d: %u\n",                                                  // 507
        "Polling is %s in device with IP %03d.",                               // 508
        "executed",                                                            // 509
        "not executed",                                                        // 510
        "Polling: reply with no request!",                                     // 511
        "Switching: reply with no request!",                                   // 512
        "Switch message was acknowledged.",                                    // 513
        "Peer with IP %03d reports input %d was set to %s.",                   // 514
        // Memory information
        "Memory information\r",                                                // 515
        "ROM-usage: %u bytes = %u%%\r",                                        // 516
        "RAM-usage:\r static variables: %u bytes = %u%%\r",                    // 517
        "Dynamic variables: %u bytes = %u%%\r",                                // 518
        "User defined stack: %u bytes reserved, %u bytes used = %u%%\r",       // 519
        "   Interrupt stack: %u bytes reserved, %u bytes used = %u%%\r",       // 520
        "\rSummary RAM: %u bytes = %u%%\r",                                    // 521
        "Get attribute %3d should not be saved.",                              // 522
        "Attribute %3d %s saved. Index: %4d",                                  // 523
        "was",                                                                 // 524
        "already",                                                             // 525
        "the relay will be switched",                                          // 526
        "Incorrect index entry",                                               // 527
        "Pop Out -> %d",                                                       // 528
        "sendMessage B %d %sM%02d#%02d",                                       // 529
        "Default parameters set",                                              // 530
        "Memory is written",                                                   // 531
        "retransmitRequest, Timer %d Event",                                   // 532
        "handleAliveNoResponseTimeOut, timer %d event",                        // 533
        "transmitAlarmRequest, timer %d event",                                // 534
        "Hardware reference error",                                            // 535
        "startUpdate",                                                           // 536
        "getUpdateStatus",                                                       // 537
        "This command start's the copyDeployUpdate",                             // 538
        "This command return the current UpdateStatus",                          // 539
        "spiFlashIf",                                                          // 540
        "Test interface for SPI flash devices",                                // 541
};
