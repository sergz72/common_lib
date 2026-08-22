mod gd32;

use std::env::args;
use std::fs;
use std::io::{Error, ErrorKind};
use crate::gd32::Gd32CRCGenerator;

trait CRCGenerator {
    fn calculate(&self, data: Vec<u8>) -> u32;
}

fn main() -> Result<(), Error> {
    let arguments: Vec<String> = args().skip(1).collect();
    let mut file_name = "".to_string();
    let mut string = "".to_string();
    let mut mode = "".to_string();
    let mut file_name_expected = false;
    let mut mode_expected = false;
    let mut string_expected = false;

    for arg in arguments {
        if string_expected {
            string = arg;
            string_expected = false;
            continue;
        }
        if file_name_expected {
            file_name = arg;
            file_name_expected = false;
            continue;
        }
        if mode_expected {
            mode = arg;
            mode_expected = false;
            continue;
        }
        if arg.starts_with("--") {
            match arg.as_str() {
                "--file" => file_name_expected = true,
                "--mode" => mode_expected = true,
                "--string" => string_expected = true,
                _ => {
                    return Err(Error::new(ErrorKind::InvalidInput,
                                          "Usage: mycrc --mode mode [--file file_name][--string string]"));
                }
            }
        }
    }
    if file_name_expected {
        return Err(Error::new(ErrorKind::InvalidInput,"file name expected"));
    }
    if mode_expected {
        return Err(Error::new(ErrorKind::InvalidInput,"mode expected"));
    }
    if string_expected {
        return Err(Error::new(ErrorKind::InvalidInput,"string expected"));
    }
    if file_name.len() == 0 && string.len() == 0 {
        return Err(Error::new(ErrorKind::InvalidInput,"file name or string must be specified"));
    }
    if mode.len() == 0 {
        return Err(Error::new(ErrorKind::InvalidInput,"missing mode parameter"));
    }
    let generator = match mode.as_str() {
        "gd32" => Gd32CRCGenerator::new(),
        _ => return Err(Error::new(ErrorKind::InvalidInput,"unknown mode"))
    };
    if string.len() != 0 {
        print_crc("string", &generator, string.into_bytes());
    }
    if file_name.len() != 0 {
        print_crc("file", &generator, fs::read(file_name)?);
    }
    Ok(())
}

fn print_crc(title: &str, generator: &Box<dyn CRCGenerator>, data: Vec<u8>) {
    let crc = generator.calculate(data);
    println!("{} crc: {:08X}", title, crc);
}