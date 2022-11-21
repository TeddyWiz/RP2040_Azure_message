/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "azure_samples.h"

/* Paste in the your IoT Hub connection string  */
const char pico_az_connectionString[] = "";

const char pico_az_x509connectionString[] = "";

const char pico_az_x509certificate[] = "";

const char pico_az_x509privatekey[] = "";

const char pico_az_id_scope[] = "0ne00677DE7";

const char pico_az_COMMON_NAME[] = "10000000d9eedbd6";

const char pico_az_CERTIFICATE[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDYjCCAkqgAwIBAgIRANjgxkzFzLYaxRp8mnIt+yYwDQYJKoZIhvcNAQELBQAw\n"
    "IDEeMBwGA1UEAwwVQVJISVNfUFJPSkVDVF9ST09UX0NBMB4XDTIyMDgxODA3NTcy\n"
    "N1oXDTMyMDgxNTA3NTcyN1owQjELMAkGA1UEBhMCLi4xCzAJBgNVBAgMAi4uMQsw\n"
    "CQYDVQQKDAIuLjEZMBcGA1UEAwwQMTAwMDAwMDBkOWVlZGJkNjCCASIwDQYJKoZI\n"
    "hvcNAQEBBQADggEPADCCAQoCggEBAMPLzzMXAUwFYQU/UkqftUqrYOSSi91x40Pw\n"
    "XdtK5Pjeh5t72L3rKZtxF8y/DrUZ96T4m+T1HCgsmbtLaTapcNdkhY4Aph2azqJK\n"
    "tNgnvEIlqdpdDuAorIk0+M2SSTKbWxA6rrfxRQbpzOEj7gDswo9oTa+vRff+Xy4K\n"
    "dYfJNxOd4tds1Xw6k2qNKQGU0tfS8il/WA5froZjpv31Tecm6OGimlnXA5cliDrn\n"
    "qZr2dcJvP9CJ+m/KWnD3MA1IYYxhWcqc4NaPn2CilbvuUaphFvepN9kbePRe+HD1\n"
    "J0x7oJTAGrpykA/TfMGCV4mVycFSqeblptbWvwDHdrUCmvdWo/cCAwEAAaN1MHMw\n"
    "HwYDVR0jBBgwFoAU3ak0pBZNDsuUT40BOtAaAEa5yRAwDAYDVR0TAQH/BAIwADAT\n"
    "BgNVHSUEDDAKBggrBgEFBQcDAjAOBgNVHQ8BAf8EBAMCB4AwHQYDVR0OBBYEFP6D\n"
    "L4LvXhdTKhlM8NxQZGMuMMRuMA0GCSqGSIb3DQEBCwUAA4IBAQBJL4LlLNbM3Pte\n"
    "HbNDa1iNwMUabxVtfpWWwWsuxzhXhoe3jFOzKLPf5ZVZ8ziCwF25dcCNk3fCfenS\n"
    "Hx8NIppHePV/YzpY5vWJCQIb3QN8cUIBVkVYBs0YleX7+iY3yzkOgbzTvodCxwXd\n"
    "+WbUUElOKIhTYllIc7BFX9fOQEHxdXmwQqsGXY1GRmwa0H7Jvl2wk544cdlX3yX1\n"
    "UP1lPeMwTvw6P70rzDiO2fyiVrCFvhQ0+gYhfHsD/mfOlI8OqK/+ObXUsdvYxury\n"
    "coTliYrQZ5UW9W3LMjNX8HR3ne4bOmjb6Mpsn4Huo94PMEnz4JCDEpkTnof0f/rW\n"
    "vAGdSXxy\n"
    "-----END CERTIFICATE-----";

const char pico_az_PRIVATE_KEY[] =
    "-----BEGIN PRIVATE KEY-----\n"
    "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDDy88zFwFMBWEF\n"
    "P1JKn7VKq2DkkovdceND8F3bSuT43oebe9i96ymbcRfMvw61Gfek+Jvk9RwoLJm7\n"
    "S2k2qXDXZIWOAKYdms6iSrTYJ7xCJanaXQ7gKKyJNPjNkkkym1sQOq638UUG6czh\n"
    "I+4A7MKPaE2vr0X3/l8uCnWHyTcTneLXbNV8OpNqjSkBlNLX0vIpf1gOX66GY6b9\n"
    "9U3nJujhoppZ1wOXJYg656ma9nXCbz/Qifpvylpw9zANSGGMYVnKnODWj59gopW7\n"
    "7lGqYRb3qTfZG3j0Xvhw9SdMe6CUwBq6cpAP03zBgleJlcnBUqnm5abW1r8Ax3a1\n"
    "Apr3VqP3AgMBAAECggEBAIcQnDFZKErh1wjAoqrZuzz0NhXXReaPvr/KG2TAKG3J\n"
    "THCkoWJ5y6zv+gQCtEmXzS6fVSM/Npo8EhySq9TKVA4xyLOpnO4FMY/gyxnlZ0eW\n"
    "JbJ9yVo2lLbdtNhSlm3zn4D1ijtXk09ujyesGm3G5Xv57sVHupOVhOEE1AjEdFrx\n"
    "tNGB40Q1VmYCuZcwDh5Cqy+45+lpUMj+G8kO2dL3cp6S/z8pIevVbALlZEoTLYec\n"
    "4oKpJRE177qyVuuPAaod0KJ4UD3726UhUPzcVZlUQhZAqNNv3t8WAoOL8gbF+LVN\n"
    "XcZ+mN6QziHfK5ZDTZFxCcGQL5W4lEY5Pi1zn7BtNLkCgYEA7xflzpZ6h+aZkHJQ\n"
    "nFojdfSQ3Y2e1eFrYp59dyjooiVILJ+MLYpTLtDisujgxGff4JmJM2P1HdtkPznf\n"
    "WuYpTzub+6dvoFoGcOMXkQSzXSax6p3KrzZWS6qeCNY4sBWl4yoyXEfEwLvnDsT0\n"
    "1RYAU3rvgz+Kc8VAOXZwI6J+BT0CgYEA0aQjoIutd99uEwfWGK5pzIAEWAb9RrwX\n"
    "OzuJDzeYv/XZAWg6WhRHAVHGizvqSR40sMaUdw3x8M9Asw8SW9kQ9Z9Vrs33ONRg\n"
    "mHEu8GhJHzafTIdsKMWq8ExJSDPW3b8BGl+ljJqbFVzGVwkM6IZAOj3EMyb/Iq2d\n"
    "fa5GA8TeqUMCgYAIDEfiAlKxjGOS5yYrfSVAbTELJB86l9Hjie4zOp9KBfM9/Ujc\n"
    "p5FRPBrFZu1Z7x0sFD74Cd9QV/gx4KLSDnlJf3oqqGIrhZw95IcAjKX09r56ZUFT\n"
    "UILrHR0gswVJeBETanIzhP7sdea7KooLOihcpwC07EieyP72cDQqHugbCQKBgBG0\n"
    "mimyAkmQfjxvOf7FpDvYSAWjOXri4ddn1NCLMoRr4BMFWYBIHCMZY/pYahYYzfxs\n"
    "GRfg/qEG8ADvce5967fC6DqmPI35KdtWG8/EDwDq3RNakKD06NX4q2vErQ33VsGC\n"
    "eHniiNyKBFpPcl6lEAGbO9nSHlQwc4+sy08ALon/AoGBAOHWlYU0uEceGBAfXXlC\n"
    "RIuUsUeaOCL0DhBC8qgeY2YmIN2wjo4fC7rokemdrpC4bzSx3mh83NX1ahCr6ZOM\n"
    "b9jTSIiURzkmDYUZVW9h0XYdH8zQEMG576WE5YiuL/HX9SHTxDthYqvLsE7cxUub\n"
    "TckSb7giFzGCICHtl8XiP+cP\n"
    "-----END PRIVATE KEY-----";
