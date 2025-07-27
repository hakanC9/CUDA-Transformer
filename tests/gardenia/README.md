# HOW TO TRANSFORM GARDENIA

## liner_base.cu file under **bc/** directory will be used as example.

1. Set ***transform_single*** key in config.txt file to absolute path of ***linear_base.cu*** in **gardenia/src/bc** directory

2. Set ***resource_dir*** key in config.txt file to absolute path of clang installation

3. Set ***includes*** key in config.txt file to absolute paths of include directories

4. Build and run the tool once. Refer to the README.md file of the main branch for building the tool.

5. Note the analyze results

6. Un-comment ***TransformerToolTest.transformSingle()*** line, re-build and re-run the tool. 

7. Transformed file will be generated under the ***temp_results*** directory. Replace the files of **gardenia/src/bc** with the transformation results 

8. Modify the **CUDA_HOME**, **CUB_DIR** and **OCL_DIR** in the ***common.mk*** file under **gardenia/src**. Example common.mk entries:

```bash
CUDA_HOME=/usr/local/cuda-12.8
CUB_DIR=/usr/local/cuda-12.8/include/cub
OCL_DIR=/usr/local/cuda-12.8/targets/x86_64-linux/include
```

9. Run:
```bash
make bc_linear_base
```

10. You can change ***optimization*** key in the config.txt file and re-do steps **6-9** without re-building the tool 


### An example config is provided