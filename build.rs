fn main() {

  let cores = num_cpus::get();
  let tpcl2 = (cores as f64).log2().floor() as u32;

  match cc::Build::new()
      .file("src/hvm.c")
      .opt_level(3)
      .warnings(false)
      .define("TPC_L2", &*tpcl2.to_string())
      .try_compile("hvm-c") {
    Ok(_) => println!("cargo:rerun-if-changed=src/hvm.c"),
    Err(e) => {
      println!("WARNING: Failed to compile hvm.c: {}", e);
      println!("Ignoring hvm.c and proceeding with build.");
    }
  }

  match cc::Build::new()
				.file("src/hvm_vk.c")
				.opt_level(3)
				.warnings(false)
				.try_compile("hvm-vk") {
						Ok(_) => {
								println!("cargo:rerun-if-changed=src/hvm_vk.c");
								println!("cargo:rustc-cfg=feature=\"vulkan\"");
								println!("cargo:rustc-link-lib=vulkan");
				},
			Err(e) => {
			  println!("sadge no vk :(");
			}
	}

  // Builds hvm.cu
  if std::process::Command::new("nvcc").arg("--version").stdout(std::process::Stdio::null()).stderr(std::process::Stdio::null()).status().is_ok() {
  
    if let Ok(cuda_path) = std::env::var("CUDA_HOME") {
      println!("cargo:rustc-link-search=native={}/lib64", cuda_path);
    } else {
      println!("cargo:rustc-link-search=native=/usr/local/cuda/lib64");
    }

    cc::Build::new()
      .cuda(true)
      .file("src/hvm.cu")
      .compile("hvm-cu");
    
    println!("cargo:rerun-if-changed=src/hvm.cu");
    println!("cargo:rustc-cfg=feature=\"cuda\"");   
  }
  else {
    println!("WARNING: CUDA compiler not found. HVM will not be able to run on GPU.");
  }

}
