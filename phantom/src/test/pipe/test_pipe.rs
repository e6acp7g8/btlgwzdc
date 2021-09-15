/*
 * The Shadow Simulator
 * See LICENSE for licensing information
 */

use test_utils::set;
use test_utils::TestEnvironment as TestEnv;

fn main() -> Result<(), String> {
    // should we restrict the tests we run?
    let filter_shadow_passing = std::env::args().any(|x| x == "--shadow-passing");
    let filter_libc_passing = std::env::args().any(|x| x == "--libc-passing");
    // should we summarize the results rather than exit on a failed test
    let summarize = std::env::args().any(|x| x == "--summarize");

    let mut tests = get_tests();
    if filter_shadow_passing {
        tests = tests
            .into_iter()
            .filter(|x| x.passing(TestEnv::Shadow))
            .collect()
    }
    if filter_libc_passing {
        tests = tests
            .into_iter()
            .filter(|x| x.passing(TestEnv::Libc))
            .collect()
    }

    test_utils::run_tests(&tests, summarize)?;

    println!("Success.");
    Ok(())
}

fn get_tests() -> Vec<test_utils::ShadowTest<(), String>> {
    let tests: Vec<test_utils::ShadowTest<_, _>> = vec![
        test_utils::ShadowTest::new("test_pipe", test_pipe, set![TestEnv::Libc, TestEnv::Shadow]),
        test_utils::ShadowTest::new(
            "test_read_write",
            test_read_write,
            set![TestEnv::Libc, TestEnv::Shadow],
        ),
        test_utils::ShadowTest::new(
            "test_large_read_write",
            test_large_read_write,
            set![TestEnv::Libc, TestEnv::Shadow],
        ),
        test_utils::ShadowTest::new(
            "test_read_write_empty",
            test_read_write_empty,
            set![TestEnv::Libc, TestEnv::Shadow],
        ),
        test_utils::ShadowTest::new("test_dup", test_dup, set![TestEnv::Libc]),
        test_utils::ShadowTest::new(
            "test_write_to_read_end",
            test_write_to_read_end,
            set![TestEnv::Libc],
        ),
        test_utils::ShadowTest::new(
            "test_read_from_write_end",
            test_read_from_write_end,
            set![TestEnv::Libc],
        ),
    ];

    tests
}

fn test_pipe() -> Result<(), String> {
    let mut fds = [0 as libc::c_int; 2];
    test_utils::check_system_call!(|| { unsafe { libc::pipe(fds.as_mut_ptr()) } }, &[])?;

    test_utils::result_assert(fds[0] > 0, "fds[0] not set")?;
    test_utils::result_assert(fds[1] > 0, "fds[1] not set")?;

    Ok(())
}

fn test_read_write() -> Result<(), String> {
    let mut fds = [0 as libc::c_int; 2];
    test_utils::check_system_call!(|| { unsafe { libc::pipe(fds.as_mut_ptr()) } }, &[])?;

    test_utils::result_assert(fds[0] > 0, "fds[0] not set")?;
    test_utils::result_assert(fds[1] > 0, "fds[1] not set")?;

    let (read_fd, write_fd) = (fds[0], fds[1]);

    test_utils::run_and_close_fds(&[write_fd, read_fd], || {
        let write_buf = [1u8, 2, 3, 4];

        let rv = test_utils::check_system_call!(
            || {
                unsafe {
                    libc::write(
                        write_fd,
                        write_buf.as_ptr() as *const libc::c_void,
                        write_buf.len(),
                    )
                }
            },
            &[]
        )?;

        test_utils::result_assert_eq(rv, 4, "Expected to write 4 bytes")?;

        let mut read_buf = [0u8; 4];

        let rv = test_utils::check_system_call!(
            || {
                unsafe {
                    libc::read(
                        read_fd,
                        read_buf.as_mut_ptr() as *mut libc::c_void,
                        read_buf.len(),
                    )
                }
            },
            &[]
        )?;

        test_utils::result_assert_eq(rv, 4, "Expected to read 4 bytes")?;

        test_utils::result_assert_eq(write_buf, read_buf, "Buffers differ")?;

        Ok(())
    })
}

fn test_large_read_write() -> Result<(), String> {
    let mut fds = [0 as libc::c_int; 2];
    test_utils::check_system_call!(|| { unsafe { libc::pipe(fds.as_mut_ptr()) } }, &[])?;

    test_utils::result_assert(fds[0] > 0, "fds[0] not set")?;
    test_utils::result_assert(fds[1] > 0, "fds[1] not set")?;

    let (read_fd, write_fd) = (fds[0], fds[1]);

    test_utils::run_and_close_fds(&[write_fd, read_fd], || {
        let mut write_buf = Vec::<u8>::with_capacity(8096 * 2);
        for _ in 0..write_buf.capacity() {
            let random_value = unsafe { libc::rand() };
            write_buf.push(random_value as u8)
        }

        let mut read_buf = Vec::<u8>::with_capacity(write_buf.len());
        read_buf.resize(read_buf.capacity(), 0);

        let mut bytes_written = 0;
        let mut bytes_read = 0;

        while bytes_read < write_buf.len() {
            let write_slice = &write_buf[bytes_written..];
            let towrite = write_slice.len();
            let rv = test_utils::check_system_call!(
                || {
                    unsafe {
                        libc::write(
                            write_fd,
                            write_slice.as_ptr() as *const libc::c_void,
                            towrite,
                        )
                    }
                },
                &[]
            )?;
            println!("Wrote {}", rv);
            bytes_written += rv as usize;

            let read_slice = &mut read_buf[bytes_read..];
            let toread = read_slice.len();
            let rv = test_utils::check_system_call!(
                || {
                    unsafe { libc::read(read_fd, read_slice.as_ptr() as *mut libc::c_void, toread) }
                },
                &[]
            )?;
            println!("Read {}", rv);
            let range_read = bytes_read..bytes_read + rv as usize;
            assert_eq!(read_buf[range_read.clone()], write_buf[range_read]);
            bytes_read += rv as usize;
        }

        Ok(())
    })
}

// pipe(2) indicates that size zero writes to pipes with O_DIRECT are no-ops,
// and somewhat implies that they are no-ops without it as well. Exerimentally
// size zero reads and writes to pipes are both no-ops.
fn test_read_write_empty() -> Result<(), String> {
    let mut fds = [0 as libc::c_int; 2];
    test_utils::check_system_call!(|| { unsafe { libc::pipe(fds.as_mut_ptr()) } }, &[])?;

    test_utils::result_assert(fds[0] > 0, "fds[0] not set")?;
    test_utils::result_assert(fds[1] > 0, "fds[1] not set")?;

    let (read_fd, write_fd) = (fds[0], fds[1]);

    test_utils::run_and_close_fds(&[write_fd, read_fd], || {
        let rv = test_utils::check_system_call!(
            || { unsafe { libc::write(write_fd, std::ptr::null(), 0,) } },
            &[]
        )?;
        test_utils::result_assert_eq(rv, 0, "Expected to write 0 bytes")?;

        let rv = test_utils::check_system_call!(
            || { unsafe { libc::read(read_fd, std::ptr::null_mut(), 0,) } },
            &[]
        )?;
        test_utils::result_assert_eq(rv, 0, "Expected to read 0 bytes")?;

        // Reading again should still succeed and not block. There are no "0
        // byte datagrams" with pipes; reading and writing 0 bytes is just a
        // no-op.
        let rv = test_utils::check_system_call!(
            || { unsafe { libc::read(read_fd, std::ptr::null_mut(), 0,) } },
            &[]
        )?;
        test_utils::result_assert_eq(rv, 0, "Expected to read 0 bytes")?;

        Ok(())
    })
}

fn test_dup() -> Result<(), String> {
    let mut fds = [0 as libc::c_int; 2];
    test_utils::check_system_call!(|| { unsafe { libc::pipe(fds.as_mut_ptr()) } }, &[])?;

    test_utils::result_assert(fds[0] > 0, "fds[0] not set")?;
    test_utils::result_assert(fds[1] > 0, "fds[1] not set")?;

    let (read_fd, write_fd) = (fds[0], fds[1]);

    // TODO: use nix instead here
    fn write(fd: libc::c_int, buf: &[u8]) -> Result<libc::ssize_t, String> {
        test_utils::check_system_call!(
            || { unsafe { libc::write(fd, buf.as_ptr() as *const libc::c_void, buf.len(),) } },
            &[]
        )
    }

    fn read(fd: libc::c_int, buf: &mut [u8]) -> Result<libc::ssize_t, String> {
        test_utils::check_system_call!(
            || { unsafe { libc::read(fd, buf.as_mut_ptr() as *mut libc::c_void, buf.len(),) } },
            &[]
        )
    }

    test_utils::run_and_close_fds(&[write_fd, read_fd], || {
        let write_fd_dup =
            test_utils::check_system_call!(|| { unsafe { libc::dup(write_fd) } }, &[])?;

        test_utils::run_and_close_fds(&[write_fd_dup], || {
            let write_buf = [1u8, 2, 3, 4];

            // write 4 bytes to original fd
            let rv = write(write_fd, &write_buf)?;
            test_utils::result_assert_eq(rv, 4, "Expected to write 4 bytes")?;

            // write 5 bytes to duped fd
            let rv = write(write_fd_dup, &write_buf)?;
            test_utils::result_assert_eq(rv, 4, "Expected to write 4 bytes")?;

            // read 8 bytes
            let mut read_buf = [0u8; 8];
            let rv = read(read_fd, &mut read_buf)?;
            test_utils::result_assert_eq(rv, 8, "Expected to read 8 bytes")?;

            test_utils::result_assert_eq(&write_buf[..], &read_buf[..4], "First 4 bytes differ")?;
            test_utils::result_assert_eq(&write_buf[..], &read_buf[4..8], "Last 4 bytes differ")?;

            Ok(())
        })
    })
}

fn test_write_to_read_end() -> Result<(), String> {
    let mut fds = [0 as libc::c_int; 2];
    test_utils::check_system_call!(|| { unsafe { libc::pipe(fds.as_mut_ptr()) } }, &[])?;

    test_utils::result_assert(fds[0] > 0, "fds[0] not set")?;
    test_utils::result_assert(fds[1] > 0, "fds[1] not set")?;

    let (read_fd, write_fd) = (fds[0], fds[1]);

    test_utils::run_and_close_fds(&[write_fd, read_fd], || {
        let write_buf = [1u8, 2, 3, 4];

        test_utils::check_system_call!(
            || {
                unsafe {
                    libc::write(
                        read_fd,
                        write_buf.as_ptr() as *const libc::c_void,
                        write_buf.len(),
                    )
                }
            },
            &[libc::EBADF]
        )?;

        Ok(())
    })
}

fn test_read_from_write_end() -> Result<(), String> {
    let mut fds = [0 as libc::c_int; 2];
    test_utils::check_system_call!(|| { unsafe { libc::pipe(fds.as_mut_ptr()) } }, &[])?;

    test_utils::result_assert(fds[0] > 0, "fds[0] not set")?;
    test_utils::result_assert(fds[1] > 0, "fds[1] not set")?;

    let (read_fd, write_fd) = (fds[0], fds[1]);

    test_utils::run_and_close_fds(&[write_fd, read_fd], || {
        let write_buf = [1u8, 2, 3, 4];

        let rv = test_utils::check_system_call!(
            || {
                unsafe {
                    libc::write(
                        write_fd,
                        write_buf.as_ptr() as *const libc::c_void,
                        write_buf.len(),
                    )
                }
            },
            &[]
        )?;

        test_utils::result_assert_eq(rv, 4, "Expected to write 4 bytes")?;

        let mut read_buf = [0u8; 4];

        test_utils::check_system_call!(
            || {
                unsafe {
                    libc::read(
                        write_fd,
                        read_buf.as_mut_ptr() as *mut libc::c_void,
                        read_buf.len(),
                    )
                }
            },
            &[libc::EBADF]
        )?;

        Ok(())
    })
}
