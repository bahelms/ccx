fn main() {
    let filename = std::env::args().nth(1).expect("No filename given");
    println!("Compiling file: {}", filename);
}
