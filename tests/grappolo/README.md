# HOW TO TRANSFORM GPU GRAPH

1. Set ***project_path*** key in config.txt file to absolute path of fixed_source_code directory

2. Set ***resource_dir*** key in config.txt file to absolute path of clang installation

3. Set ***includes*** key in config.txt file to absolute paths of include directories

4. Build and run the tool once. Refer to the README.md file of the main branch for building the tool.

5. Note the analyze results

6. For each optimization to apply, choose options accordingly to the **OptimizationTypes.txt** file and write it to the ***optimization*** key in config.txt file. If a target file does not contain any appliable optimization, it's analyze result will be empty string. Printing results are not a part of the tool, so it is printed as -1 to the console for convenience. Only provide optimization values for the files that contain any possible optimization.
```bash
22
-1
167
```

For an example output like this, **11111** value for the ***optimization*** key in config will be read as **"Apply 11 and 111 optimizations for the first and third file"**. 

7. Un-comment ***TransformerToolTest.transformOnly()*** line, re-build and re-run the tool. 

8. Transformed files will be generated under the ***temp_results*** directory. Replace the files of **grappolo/gpu_graph** with the transformation results 

9. Create ***datasets*** and ***bin*** directories inside **grappolo/**

10. Copy karate.graph file from **grappolo/** to **grappolo/datasets**

11. Simply run ***cmake*** and ***make*** commands

12. Run ***./gpu_graph*** under **gpu_graph/** an example command will be like this: 
```bash
./gpu_graph -p 100  -f 5 ../../datasets/karate.graph -b 0 -o >> transformed_results.txt
```

13. You can change ***optimization*** key in the config.txt file and re-do steps **6-12** without re-building the tool 

### An example config is provided