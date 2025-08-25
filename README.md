# rgf
A C89 standard compliant, single header, nostdlib (no C Standard Library) raw geometry format (RGF).

<p align="center">
<a href="https://github.com/nickscha/rgf"><img src="assets/rgf.png"></a>
</p>

For more information please look at the "rgf.h" file or take a look at the "examples" or "tests" folder.

> [!WARNING]
> THIS PROJECT IS A WORK IN PROGRESS! ANYTHING CAN CHANGE AT ANY MOMENT WITHOUT ANY NOTICE! USE THIS PROJECT AT YOUR OWN RISK!

## Quick Start

Download or clone rgf.h and include it in your project.

```C
#include "rgf.h"             /* Raw Geometry Format                          */
#include "rgf_platform_io.h" /* Optional: OS-Specific read/write file IO API */

int main() {

    /* For simplicity we assign some stack memory for storing vertices and indices data */
    float vertices_buffer[30000];
    int   indices_buffer[60000];
    float normals_buffer[30000];

    /* The binary buffer is used for reading/writing files and encoding/decoding */
    #define BINARY_BUFFER_CAPACITY 1500000
    unsigned char binary_buffer[BINARY_BUFFER_CAPACITY];
    unsigned long binary_buffer_size = 0;

    rgf_model model = {0};
    model.vertices  = vertices_buffer;
    model.indices   = indices_buffer;
    model.normals   = normals_buffer;

    /* Just for demonstration we use a small utility to read a file (nostdlib but platform specific) */
    if (!rgf_platform_read("head.obj", binary_buffer, BINARY_BUFFER_CAPACITY, &binary_buffer_size)) {
        return 1;
    }

    /* Parses the obj file into the rgf data model */
    if (!rgf_parse_obj(&model, binary_buffer, binary_buffer_size)) {
        return 1;
    }

    /* ########################################################## */
    /* # Geometry manipulation functions                          */
    /* ########################################################## */
    /* Center the model to a specified x,y,z position */
    rgf_model_center(&model, 0.0f, 0.0f, 0.0f);

    /* Scale the model to a unit model (1 x 1 x 1) */
    rgf_model_scale(&model, 1.0f);

    /* Reset the scale back to the original */
    rgf_model_scale_reset(&model);

    /* Reset the model back to its original position */
    rgf_model_center_reset(&model);

    /* Calculate the normals */
    rgf_model_calculate_normals(&model);

    /* ########################################################## */
    /* # Encode/Decode rgf_model to binary                        */
    /* ########################################################## */
    {
        rgf_model binary_model = {0};

        /* Encode the rgf_model into a binary rgf */
        if (!rgf_binary_encode(binary_buffer, BINARY_BUFFER_CAPACITY, &binary_buffer_size, &model)) {
            return 1;
        }

        /* Decode the binary_buffer back to the rgf_model */
        if (!rgf_binary_decode(binary_buffer, binary_buffer_size, &binary_model)) {
            return 1;
        }

        /* Example check of decoded version vs original rgf_model */
        if (binary_model.vertices_size != model.vertices_size ||
            binary_model.indices_size  != model.indices_size
        ) {
            return 1;
        }
    }

    return 0;
}
```

## Run Example: nostdlib, freestsanding

In this repo you will find the "examples/rgf_win32_nostdlib.c" with the corresponding "build.bat" file which
creates an executable only linked to "kernel32" and is not using the C standard library and executes the program afterwards.

## "nostdlib" Motivation & Purpose

nostdlib is a lightweight, minimalistic approach to C development that removes dependencies on the standard library. The motivation behind this project is to provide developers with greater control over their code by eliminating unnecessary overhead, reducing binary size, and enabling deployment in resource-constrained environments.

Many modern development environments rely heavily on the standard library, which, while convenient, introduces unnecessary bloat, security risks, and unpredictable dependencies. nostdlib aims to give developers fine-grained control over memory management, execution flow, and system calls by working directly with the underlying platform.

### Benefits

#### Minimal overhead
By removing the standard library, nostdlib significantly reduces runtime overhead, allowing for faster execution and smaller binary sizes.

#### Increased security
Standard libraries often include unnecessary functions that increase the attack surface of an application. nostdlib mitigates security risks by removing unused and potentially vulnerable components.

#### Reduced binary size
Without linking to the standard library, binaries are smaller, making them ideal for embedded systems, bootloaders, and operating systems where storage is limited.

#### Enhanced performance
Direct control over system calls and memory management leads to performance gains by eliminating abstraction layers imposed by standard libraries.

#### Better portability
By relying only on fundamental system interfaces, nostdlib allows for easier porting across different platforms without worrying about standard library availability.
