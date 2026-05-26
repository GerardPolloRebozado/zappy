use nix::libc;
use std::os::raw::c_int;
use std::sync::atomic::{AtomicUsize, Ordering};

pub static SIGNAL_RECEIVED: AtomicUsize = AtomicUsize::new(0);

pub fn install_sigint_handler() {
    #[cfg(unix)]
    unsafe {
        signal(2, handle_sigint);
    }
}

#[cfg(unix)]
unsafe extern "C" {
    fn signal(sig: c_int, handler: extern "C" fn(c_int)) -> usize;
}

extern "C" fn handle_sigint(sig: c_int) {
    if sig == libc::SIGINT {
        SIGNAL_RECEIVED.fetch_add(1, Ordering::SeqCst);
    }
}
