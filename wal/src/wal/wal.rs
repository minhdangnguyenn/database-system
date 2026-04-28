use std::fs::{File, OpenOptions};
use std::io::{Read, Seek, SeekFrom, Write};
use std::path::PathBuf;

pub struct Wal {
    file: File,
    path: PathBuf,
}

impl Wal {
    pub fn open(path: &str) -> Self {
        let path = PathBuf::from(path);
        let file = OpenOptions::new()
            .read(true)
            .write(true)
            .create(true)
            .open(&path)
            .unwrap();
        Self { file, path }
    }

    pub fn read(&mut self) -> Vec<u8> {
        self.file.seek(SeekFrom::Start(0)).unwrap();
        let mut data = Vec::new();
        self.file.read_to_end(&mut data).unwrap();
        data
    }

    pub fn append(&mut self, data: &[u8]) -> std::io::Result<()> {
        self.file.seek(SeekFrom::End(0))?;
        self.file.write_all(data)?;
        self.file.flush()?;
        Ok(())
    }

    pub fn replay(&mut self) -> std::io::Result<()> {
        self.file.seek(SeekFrom::Start(0))?;
        let mut data = Vec::new();
        self.file.read_to_end(&mut data)?;

        Ok(())
    }
}
