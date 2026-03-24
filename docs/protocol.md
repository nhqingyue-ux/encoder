# Encoder → Host Communication Protocol

## Overview

The rotary encoder board sends input events to the host controller via UART.
This is a **one-way** (encoder → host) ASCII protocol designed for simplicity
and debuggability.

## Physical Layer

| Parameter | Value |
|-----------|-------|
| Interface | UART (USART3 on STM32G070) |
| Baud rate | 115200 |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |
| Flow ctrl | None |
| TX pin | PB8 (USART3_TX, AF4) |
| RX pin | PB9 (USART3_RX, AF4) |
| Direction | Encoder → Host (TX only, RX reserved) |

> **Note**: In debug builds (`-Og -DDEBUG`), USART3 also carries debug log
> messages (always start with `[`). In release builds (`-Os -DNDEBUG`),
> debug logging is compiled out — USART3 carries **only** protocol messages.
> The host parser should still ignore unknown bytes for robustness.

## Message Format

Each message is a **single ASCII character** followed by a newline (`\n`, 0x0A).

```
<CMD>\n
```

No header, no length, no CRC. The newline serves as the message delimiter.

## Commands

### Encoder → Host

| Byte | ASCII | Meaning | Trigger |
|------|-------|---------|---------|
| 0x52 | `R` | Right | Clockwise rotation (one detent) |
| 0x4C | `L` | Left | Counter-clockwise rotation (one detent) |
| 0x59 | `Y` | Yes | Button press (falling edge) |

### Host → Encoder

| Byte | ASCII | Meaning | Response |
|------|-------|---------|----------|
| 0x3F | `?` | Ping / health check | `OK\n` |

The ping command allows the host to verify that the encoder is alive and
the UART link is working. Send `?` (with or without `\n`); the encoder
replies `OK\n` within one main-loop cycle (~1ms).

## Timing

- Messages are sent **immediately** on input events.
- Minimum interval between messages: ~10ms (limited by encoder detent speed).
- No periodic heartbeat or keep-alive.

## Examples

```
Encoder rotated clockwise 3 clicks, then button pressed:

  R\n
  R\n
  R\n
  Y\n

Encoder rotated counter-clockwise 2 clicks:

  L\n
  L\n
```

## Host Parsing (Pseudocode)

```c
// Receive events from encoder
char c = uart_read_byte();
switch (c) {
    case 'R': handle_right(); break;
    case 'L': handle_left();  break;
    case 'Y': handle_yes();   break;
    case '\n': break;  // delimiter, ignore
    default:   break;  // unknown, ignore
}

// Health check (send once on startup or periodically)
uart_write('?');
char resp[4];
uart_read_line(resp, sizeof(resp), timeout_ms=100);
if (strcmp(resp, "OK") == 0) {
    // encoder is alive
}
```

## Error Recovery

- If a byte is corrupted or lost, the `\n` delimiter ensures the next
  message will be correctly framed.
- Unknown bytes should be silently discarded.
- No ACK/NAK mechanism (fire-and-forget).

## Future Extensions

If needed, the protocol can be extended to:

```
<CMD>:<PARAM>\n
```

For example:
- `R:3\n` — rotated right by 3 detents (batch mode)
- `P:1\n` — button pressed (1=down, 0=up, for long-press detection)

This remains backward-compatible: existing parsers ignore the `:PARAM` part
since they only check the first byte.

## Wiring Diagram

```
  Encoder Board            Host Board
  ┌──────────┐             ┌──────────┐
  │ PB8  (TX)│────────────→│ RX       │
  │ PB9  (RX)│←────────────│ TX       │  (reserved, unused)
  │ GND      │─────────────│ GND      │
  └──────────┘             └──────────┘
```

> **Important**: GND must be connected between the two boards.
