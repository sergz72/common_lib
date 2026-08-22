use crate::CRCGenerator;

const POLY: u32 = 0x04C11DB7;

pub struct Gd32CRCGenerator {
}

fn calc_crc(mut crc: u32, v: u32) -> u32 {
    crc ^= v;
    for _ in 0..32 {
        crc = if crc & 0x80000000 != 0 {
            ((crc << 1) ^ crate::gd32::POLY) & 0xFFFFFFFF
        } else {
            (crc << 1) & 0xFFFFFFFF
        }
    }
    crc
}

impl CRCGenerator for Gd32CRCGenerator {
    fn calculate(&self, data: Vec<u8>) -> u32 {
        let mut crc = 0xFFFFFFFF;

        let mut l = data.len();
        let mut idx = 0;
        while l >= 4 {
            let v = u32::from_le_bytes(data[idx..idx+4].try_into().unwrap());
            idx += 4;
            l -= 4;
            crc = calc_crc(crc, v);
        }
        if l != 0 {
            let v = match l {
                1 => data[idx] as u32,
                2 => u16::from_le_bytes(data[idx..idx+2].try_into().unwrap()) as u32,
                3 => u32::from_le_bytes([data[idx], data[idx+1], data[idx+2], 0].try_into().unwrap()),
                _ => unreachable!(),
            };
            crc = calc_crc(crc, v);
        }
        crc
    }
}

impl Gd32CRCGenerator {
    pub fn new() -> Box<dyn CRCGenerator> {
        Box::new(Self {})
    }
}