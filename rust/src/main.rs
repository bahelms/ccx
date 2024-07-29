use std::{fs::File, io::BufReader};

fn main() {
    let filename = std::env::args().nth(1).expect("Error: No filename given");
    let stage: i32 = std::env::args()
        .nth(2)
        .expect("Error: No stage given")
        .parse()
        .expect("Error: Stage must be an integer");
    let file = File::open(&filename).expect("File not found");
    let reader = BufReader::new(file);

    ccx::compile(reader, filename, stage);
}
