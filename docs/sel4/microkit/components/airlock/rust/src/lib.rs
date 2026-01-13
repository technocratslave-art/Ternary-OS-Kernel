#![no_std]

use core::sync::atomic::{AtomicBool, Ordering};

static VIOLATION: AtomicBool = AtomicBool::new(false);

// MVP “needle hashes” (32 bytes each, hex-decoded offline in future step)
// For now: placeholder patterns in ASCII to prove plumbing.
const NEEDLES: [&[u8]; 2] = [
    b"NEEDLE_HASH_EXAMPLE_1",
    b"NEEDLE_HASH_EXAMPLE_2",
];

// Simple “sensitive” pattern examples (MVP):
// - anything that looks like a PEM header
// - anything that looks like "PRIVATE_KEY"
// - anything that matches a needle marker
fn is_sensitive_window(window: &[u8]) -> bool {
    if window.starts_with(b"-----BEGIN") { return true; }
    if window.windows(11).any(|w| w == b"PRIVATE_KEY") { return true; }
    for n in NEEDLES.iter() {
        if window.windows(n.len()).any(|w| w == *n) { return true; }
    }
    false
}

/// Scrub in place. Returns number of bytes replaced.
/// Sets VIOLATION if needle signature detected (for MVP, treated as fatal).
#[no_mangle]
pub extern "C" fn airlock_scrub_inplace(buf: *mut u8, len: u32) -> u32 {
    VIOLATION.store(false, Ordering::Relaxed);

    if buf.is_null() || len == 0 { return 0; }
    let s = unsafe { core::slice::from_raw_parts_mut(buf, len as usize) };

    let mut replaced: u32 = 0;

    // Sliding scan; redact by overwriting bytes with '*'
    // MVP: replace any window that hits sensitive patterns.
    // Later: switch to structured token scanning + hash-based detection.
    let scan_len = s.len().min(2048); // bound work for deterministic timing
    for i in 0..scan_len {
        let end = (i + 64).min(scan_len);
        let window = &s[i..end];

        if is_sensitive_window(window) {
            // If needle detected -> violation (hard boundary)
            for n in NEEDLES.iter() {
                if window.windows(n.len()).any(|w| w == *n) {
                    VIOLATION.store(true, Ordering::Relaxed);
                }
            }
            // Redact this byte
            s[i] = b'*';
            replaced += 1;
        }
    }

    replaced
}

#[no_mangle]
pub extern "C" fn airlock_detected_violation() -> u32 {
    if VIOLATION.load(Ordering::Relaxed) { 1 } else { 0 }
}
