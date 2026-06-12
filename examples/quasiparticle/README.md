# Tutorial: Quasiparticle Propagation in G4CMP

This example is a demonstration of several features and physics processes that are new to G4CMP as of November 2025. Ryan Linehan, linehan3@fnal.gov made this, so please email him with questions/compliments/complaints.

## Preliminaries

To run this example, you need to install ROOT, Geant4 (Geant4-v11.4.1), and G4CMP (a version at least as large as G4CMP-V10, but we recommend G4CMP-V10-03-00). Our recommended installation method can be found on the [RISQ 2026 Confluence Landing Page](https://confluence.slac.stanford.edu/spaces/G4CMP/pages/711111279/RISQ+2026+G4CMP+Workshop), which contains instructions on how to set up Apptainer, which will give you a standardized software environment compatible with this tutorial. However, we here repeat setup tips included in the last RISQ Tutorial example, for those who prefer to manually set up their environment.

> [!CAUTION]
> If you do not update your local G4CMP copy to _at least_ G4CMP-V10-00-00, you will be completely unable to run this example. Please pull and install the most recent version of `master`. You can technically run with Geant4-v10.7.0 or later, but your output will not perfectly match what we get in this tutorial.

### Tips for Manually Installing Geant4 and G4CMP
While installing ROOT, Geant4, or G4CMP, a good practice is to assign to each package three directories: a source directory `XXXXX`, a build directory `XXXXX-build`, and an install directory `XXXXX-install`. On my machine, the base name (`XXXXX`) for my Geant4 build is `geant4-v11.4.1`, and the base name for the G4CMP build is `G4CMP_quasiparticle`. While the installation instructions for these can be found in the packages' documentation, it's worth reminding that the last steps in the process of each of these installations should be to run `make` and `make install` while in the `XXXXX-build` directory. Moreover, we recommend installing Geant4 with the cmake flags `-DGEANT4_INSTALL_DATA=ON` and `-DGEANT4_USE_OPENGL_X11=ON`. In particular, the OpenGL flag will enable visualization, which we will frequently use. However, if you can successfully run other visualizers like DAWN, those are also perfectly fine.

### Setting up environment
Assuming you've built these directories and you're opening up a new terminal, you'll need to source the environmental setup scripts for these:
```
source /path/to/geant4-v11.4.1-install/bin/geant4.sh
source /path/to/G4CMP_quasiparticle-install/share/G4CMP/g4cmp_env.sh
```
Now we can compile our example. Copy this tutorial's source directory into a new directory -- I like to copy it outside of the whole G4CMP source directory just to avoid confusion and remember that this is its own executable that needs to be made. Moreover, make build and install directories to accompany it:
```
cd /path/to/G4CMP_quasiparticle 
cp -r ./examples/quasiparticle /path/to/
cd /path/to/
mkdir quasiparticle-build
mkdir quasiparticle-install
```
Now we head into our build directory and run CMake:
```
cd quasiparticle-build
cmake -DCMAKE_INSTALL_PREFIX=/path/to/quasiparticle-install -DCMAKE_CXX_STANDARD=17 ../quasiparticle/
```
If this runs successfully, we should be able to run make and then make install, and we're done:
```
make
make install
```
If those build without errors, we should be ready to get started.

## New Physics Processes 

The physics involved in this example is an extension of the basic physics in G4CMP intended to expand use to a broader set of applications, and includes several new processes as shown in the following diagram. In this section, we'll have a conceptual discussion of these processes as applied to phonons and BogoliubovQPs. 

<img width="951" height="274" alt="NewPhysicsProcesses_ForGithub1" src="https://github.com/user-attachments/assets/4ad12468-fd62-43f9-9cfd-5afbb142c5fc" />


### New/Updated Phonon processes Implemented in G4CMP-v10

1. Phonon transmission through surfaces: `G4CMPPhononBoundaryProcess.cc`
	* The physics involved in transmitting phonons through the boundary follows the same logic in `G4CMPBoundaryUtils::ApplyBoundaryAction`, but now in the case that all of absorption-at-electrode (i.e. KaplanQP), "simple" absorption, and reflection fail to trigger, the phonon will transmit through the interface. You can test this by turning off KaplanQP and setting `pAbsProb` and `pReflProb` both to values less than 1.0.
	* Boundary surfaces must be applied in both directions for a given interface.
 	* Currently, no "physics" is done at these interfaces -- phonons pass straight through if they pass through. Proper phonon refraction based on acoustic properties, etc, is an ongoing project.
  	* Specular vs. diffuse reflection is handled as it was in the previous version.
   	* Notably, phonons in thin films travel in the full 3-dimensional space (to be contrasted with QPs, next)
2. Phonon Polycrystalline Grain Boundary Scattering: `G4CMPPhononPolycrystalElasticScattering.cc`
	* This is just an elastic scattering that redirects the phonon after drawing a next step based on a characteristic length. The length represents the characteristic grain boundary size in a polycrystal.
3. Cooper-pair breaking by phonons: `G4CMPSCPairBreakingProcess.cc`
	* The rate of this is dictated by a combination of parameters set in the `CrystalMaps/Al/config.txt` file and parameters passed into the LatticePhysical attached to a volume. See more in the geometry construction section of the tutorial below.
	* This will produce two BogoliubovQP particles from a phonon above 2Δ.

### BogoliubovQPs and BogoliubovQP Processes Implemented in G4CMP-v10 

G4CMP-v10 adds a particle definition for a BogoliubovQP (Bogoliubov Quasiparticle) to the list of trackable objects. While these QPs can exist anywhere in a thin superconducting film volume, their transport is currently handled only in 2D on computational efficiency grounds, even while phonons can transport in full 3D space. Right now, the two dimensions that these can exist in are _specifically_ the World XY frame, but this limitation will be softened with updates in the coming year. Continuing with numbers corresponding to the bubbles labeled in the above figure,

4. QP Diffusion: `G4CMPQPDiffusion.cc`
 	* This is a doozy of a function. It uses an efficient MC [approach to diffusion](https://arxiv.org/pdf/1304.7807) based on an algorithm called [Walk-on-Spheres](https://en.wikipedia.org/wiki/Walk-on-spheres_method) to do diffusion steps of QPs in thin films. Currently only implemented in 2D, and moreover only currently implemented in XY specifically.
  	* For fine geometries (like coplanar waveguides), this will take some time to run. The execution time is dependent on the relationship between typical length scales traveled before hitting a boundary and the overall lifetime of the QP (either via recombination, absorption, or local trapping).
  	* If you intend to have bonafide, trackable BogoliubovQPs in your simulation, this must be turned on for _anything_ to be accurate.
   	* There is also a "secondary" diffusion process, `G4CMPQPDiffusionTimeStepperProcess.cc`, that can be used in conjunction with `G4CMPQPDiffusionProcess.cc` to force finer diffusion steps that are not determined by local geometry. This is useful for testing, as we will explore later in this tutorial.
5. Phonon radiation by QPs: `G4CMPQPRadiatesPhononProcess.cc`
	* This will radiate phonons from QPs above delta. The rate is affected by a similar set of parameters to the `G4CMPSCPairBreaking` process.
6. QP Recombination: `G4CMPQPRecombinationProcess.cc`
	* This will take a QP and "recombine" it with an ambient quasiparticle that is implicitly in the environment due to some ambient density. A phonon will emerge half of the time. This strategy is a kludge to conserve energy.
	* This does *not* do n^2 recombination. This recombination is linear in the density of quasiparticles and is a good approximation in the limit of low density of QPs. We'll put back-of-the-envelope numbers to this regime soon. Again, this does *not* do n^2 recombination.
	* The rate is affected by a similar set of parameters to the `G4CMPSCPairBreaking` process.
7. Quasiparticle boundary interactions: `G4CMPQPBoundaryProcess.cc`
	*  `G4CMPSurfaceProperties` now has two additional parameters: `qpAbsProb` (QP Absorption Probability) and `qpReflProb`, which come after the charge and phonon values.
 	*  When you define a superconducting volume, you will have to define a superconducting gap value for that volume's LatticePhysical. If a BogoliubovQP impinges upon a superconductor whose gap is larger than the QP's energy, it will reflect with 100% probability, ignoring the `qpReflProb` you set. If the QP energy is above the gap of the new superconductor, then you transmit with 100% probability unless you have specified a nonzero `qpAbsProb` or `qpReflProb`.
8. QP Local Trapping: `QPLocalTrappingProcess.cc`
	* This is a generic linear loss term that kills QPs after they exist for some characteristic lifetime. Notionally this is from trapping on shallow trapping sites, and does not generate phonons.
   	* The rate of this is dependent on a dedicated singular superconductor parameter.

> [!NOTE]
> For those interested in a look under the hood, you can take a look at which processes are implemented for which particles in G4CMPPhysics.cc.

### Table of New Processes' Parameters

There are a set of "superconducting" parameters needed to describe the behavior of QPs and phonon-QP interactions. Below is a table showing these parameters. A few notes:
* The first two of these parameters, `sc_tau0_qp` and `sc_tau0_ph`, are defined in a superconducting material's `config.txt` file. This is done because these parameters are more material-intrinsic than the rest, which may vary depending on the thickness, location, purity, etc., of a physical lattice. As a user, you should ideally not need to modify these parameters.
* The rest of these parameters can vary from superconducting volume to volume on a given chip depending on thickness, location, etc., and so we require that you set these for each superconductor volume you define, via the volume's associated `G4LatticePhysical` object.
* With the table we have provided _example_ values, which are approximately useful values for a boilerplate aluminum film.

| Parameter Name  | Description | Location of Definition | Example Value | Processes Affected |
| ------------- | ------------- | ------------- | ------------- | ------------- |
| `sc_tau0_qp`  | Characteristic QP Lifetime | `CrystalMaps/Al/config.txt` | 438 ns | `G4CMPSCPairBreakingProcess.cc`, `G4CMPQPRecombinationProcess.cc`, `G4CMPQPRadiatesPhononProcess.cc` |
| `sc_tau0_ph` | Characteristic Phonon Lifetime  | `CrystalMaps/Al/config.txt` | 0.242 ns | `G4CMPSCPairBreakingProcess.cc`, `G4CMPQPRecombinationProcess.cc`, `G4CMPQPRadiatesPhononProcess.cc` |
| `polycryElScatMFP` | Characteristic Polycrystalline Grain Boundary Scattering Length | Second argument of `G4LatticePhysical` constructor | 30 nm | `G4CMPPhononPolycrystalElasticScattering.cc` |
| `scDelta0` | Zero-Temperature Superconducting Gap, Δ | Third argument of `G4LatticePhysical` constructor | 180 μeV | `G4CMPSCPairBreakingProcess.cc`, `G4CMPQPRecombinationProcess.cc`, `G4CMPQPRadiatesPhononProcess.cc`, `G4CMPQPBoundaryProcess.cc`, `G4CMPQPDiffusion.cc` |
| `scTeff` | Effective Temperature | Fourth argument of `G4LatticePhysical` constructor | 0.2 K | `G4CMPSCPairBreakingProcess.cc`, `G4CMPQPRecombinationProcess.cc`, `G4CMPQPRadiatesPhononProcess.cc`, `G4CMPQPBoundaryProcess.cc`, `G4CMPQPDiffusion.cc` |
| `scDn` | Normal-state QP Diffusion Constant | Fifth argument of `G4LatticePhysical` constructor | 6 μm^{2} / ns | `G4CMPQPDiffusion.cc` |
| `scTauQPTrap` | Characteristic QP Local Trapping Time | Sixth argument of `G4LatticePhysical` constructor | 1 ms | `G4CMPQPLocalTrappingProcess.cc` | 


### When should you tailor your simulation to use these processes?

An important question to ask about building or running _any_ simulation is, "what advantage do I get from running it?" On one hand, undermodeling a system may make it challenging to fit important features of experimental spectra or time series. On the other hand, it is also possible to overmodel a system by providing more parameters than needed, sweeping over an excessively large parameter space, overfitting, and wasting computational resources in doing so. So how do you strike a balance? 

Rather than being prescriptive about what applications can make best use of this new addition, we propose flavors of problem that this new version can now model better. These include devices whose signal or performance is reasonably dependent on:
* The locations and energy distribution of quasiparticles produced in an interaction
* The transport of quasiparticles across regions of varying superconducting gap Δ.
* Phonon recycling (i.e. QP recombination into phonons that rebreak Cooper pairs)
Some examples of these include superconducting qubit devices, where diffusion plays a role in governing the population of QPs that can make it to the junction for tunneling, as well as resonators, where different densities of QPs at different points on the resonator may produce variable signals. We'll look at an example of the latter in our Tutorial Example 2.

However, if your application does not need to model these effects, a more limited superconducting response is still handled by the KaplanQP class within G4CMP, and executes significantly faster. This works relatively well for spatially limited devices, but currently ignores QP recombination.

## Tutorial Example 1: Geometry Construction

The goal of this section is to introduce advanced users to new elements of geometry construction that are needed for adopting the new features of G4CMP-V10. We're going to start in approximately the same place as the original RISQ Tutorial example: by inspecting a moderately complicated superconducting qubit geometry, once again based on the "candlestick" Xmon qubits designed by the McDermott Group at UW-Madison. The following features are on this chip:
* Silicon chip substrate
* Aluminum ground plane (sibling to the silicon chip in the geometry heirarchy)
* Aluminum coplanar waveguide (CPW) transmission line with wirebond pads (daughter of the ground plane)
* Aluminum coplanar waveguide (CPW) quarter-wave resonators (x6) (daughters of the ground plane)
* Aluminum Xmon qubits (no junction leads included) (also daughters of the ground plane)
* Copper mounts for thermalization (sibling to the silicon chip in the geometry heirarchy)
  
To see these features, we'll run our first macro, `quasiparticle_geometry_vis.mac`, from our `quasiparticle-build` directory. First, we'll navigate there and boot up G4CMP:
```
cd /path/to/quasiparticle-build
./g4cmpQuasiparticle
```
(Note that the binary, `g4cmpQuasiparticle`, may also show up in `../quasiparticle-install/bin/`, so if it doesn't appear in your `quasiparticle-build` directory, you can check there.) This will start an interactive session, and now we can run our macro. If you've followed the installation instructions above and kept `quasiparticle` and `quasiparticle-build` separate at the same level of the filetree, then you can just run
```
/control/execute ../quasiparticle/G4Macros/quasiparticle_geometry_vis.mac
```
which should give you the following result, which we'll reference in the several following sections on geometry construction:

<img width="591" height="597" alt="image" src="https://github.com/user-attachments/assets/31d1928d-6ab5-4852-9afc-9ba29648e8b8" />

To understand what about this geometry is actually new, let's explore some of the new physics. We're going to simulate a low-energy acoustic phonon and watch it bounce around the device a bit. You can go ahead and close the session with `exit` and then let's open the macro with (y)our favorite text editor
```
emacs ../quasiparticle/G4Macros/quasiparticle_geometry_vis.mac
```
Let's first look at phonon propagation. We'll make three changes to this macro. First, uncomment these three lines:
```
/vis/filtering/trajectories/particleFilter-0/add phononTS
/vis/filtering/trajectories/particleFilter-0/add phononTF
/vis/filtering/trajectories/particleFilter-0/add phononL
```
to allow the visualization to no longer filter out phonons when displaying tracks. Second, let's turn off Cooper-pair breaking so that when we allow phonons to enter our superconducting film, they just pass straight through without creating QPs, which may visually confuse the view. We'll also turn off phonon polycrystalline elastic scattering, just to simplify things as well.
```
#/process/inactivate phononScattering
#/process/inactivate phononDownconversion
/process/inactivate phononPolycrystalElasticScattering
#/process/inactivate qpRecombination
#/process/inactivate qpRadiatesPhonon
/process/inactivate scPairBreaking
#/process/inactivate qpDiffusion
#/process/inactivate qpLocalTrapping
#/process/inactivate qpDiffusionTimeStepper
```
Third, let's set the phonon bounces to 100, so that we have ample chance of watching the phonons do interesting things in the film and substrate.
```
/g4cmp/phononBounces 100
```
Okay with that, let's rerun. 
```
./g4cmpQuasiparticle
/control/execute ../quasiparticle/G4Macros/quasiparticle_geometry_vis.mac
```
You should see this frame:

<img width="589" height="591" alt="image" src="https://github.com/user-attachments/assets/7a70ae9a-d6b5-4307-b634-9a1990209628" />

Showing a single phonon (here, all polarizations are green for simplicity) bouncing around our device. To see how this is different from prior versions of G4CMP, let's take a look at the verbose tracking output, which should have also been printed by this macro:

```
*********************************************************************************************************
* G4Track Information:   Particle = phononTF,   Track ID = 1,   Parent ID = 0
*********************************************************************************************************

Step#    X(mm)    Y(mm)    Z(mm) KinE(MeV)  dE(MeV) StepLeng TrackLeng  NextVolume ProcName
    0    -2.09        1      4.9     4e-09        0        0         0 SiliconChip initStep
    1    -2.04     0.93     4.62     4e-09        0    0.294     0.294       World Transportation
    2    -2.04     0.93     4.62     4e-09        0        0     0.294 SiliconChip Transportation
    3    -2.39    0.926        5     4e-09        0    0.516      0.81 GroundPlane Transportation
    4    -2.39    0.926        5     4e-09        0 0.000122      0.81       World Transportation
    5    -2.39    0.926        5     4e-09        0        0      0.81 GroundPlane Transportation
    6    -2.39    0.926        5     4e-09        0 0.000553     0.811 SiliconChip Transportation
    7    0.703   -0.544     4.81     4e-09        0     3.43      4.24 SiliconChip phononScattering
    8    0.671    -0.25     4.62     4e-09        0     0.35      4.59       World Transportation
    9    0.671    -0.25     4.62     4e-09        0        0      4.59 SiliconChip Transportation
   10    0.668   -0.277     4.66     4e-09        0   0.0509      4.64 SiliconChip phononScattering
   11    0.615   -0.327     4.62     4e-09        0   0.0847      4.73       World Transportation
   12    0.615   -0.327     4.62     4e-09        0        0      4.73 SiliconChip Transportation
   13     0.52   -0.269        5     4e-09        0    0.397      5.12 GroundPlane Transportation
   14     0.52   -0.269        5     4e-09        0 9.77e-05      5.12       World Transportation
   15     0.52   -0.269        5     4e-09        0        0      5.12 GroundPlane Transportation
   16     0.52   -0.269        5     4e-09        0 0.000185      5.12 SiliconChip Transportation
   17    0.361    -1.22     4.62     4e-09        0     1.03      6.16       World Transportation
   18    0.361    -1.22     4.62     4e-09        0        0      6.16 SiliconChip Transportation
   19     1.06    -1.15     4.94     4e-09        0     0.77      6.93 SiliconChip phononScattering
   20     1.06    -1.14     4.95     4e-09        0   0.0126      6.94 SiliconChip phononScattering
```
Only the first twenty steps are shown, and show a quasi-periodic behavior in which the phonon's volume follows a trajectory like SiliconChip-->World-->SiliconChip-->GroundPlane-->World-->GroundPlane-->SiliconChip. This cadence demonstrates a phonon reflecting off of the vacuum boundaries with the world (the steps after "World" are zero-length turnaround steps) and then transmitting through the SiliconChip-to-Groundplane interface. Here the StepLeng (step length) column for the steps like Step 4, in which the last step's NextVolume was the GroundPlane and the current step's NextVolume is World, shows that the phonon is propagating around 0.000122 mm (about 122 nm), which is comparable to the thickness of the ground plane. Together, these things demonstrate that _the ground plane is a physically realized volume in which the phonons can propagate_, which is a new feature of this version of G4CMP.

With that simple phonon example under our belt, let's make things a bit more complicated and see what happens when we turn on quasiparticle physics. Go ahead and exit (`exit`), and let's re-enter our macro, reactivate Cooper-pair breaking:

```
#/process/inactivate phononScattering
#/process/inactivate phononDownconversion
/process/inactivate phononPolycrystalElasticScattering
#/process/inactivate qpRecombination
#/process/inactivate qpRadiatesPhonon
#/process/inactivate scPairBreaking
#/process/inactivate qpDiffusion
#/process/inactivate qpLocalTrapping
#/process/inactivate qpDiffusionTimeStepper
```
and set the trajectory visualization to ignore phonons but now no longer ignore quasiparticles:
```
# Trajectory filtering by particle type
/vis/filtering/trajectories/create/particleFilter
#/vis/filtering/trajectories/particleFilter-0/add phononTS
#/vis/filtering/trajectories/particleFilter-0/add phononTF
#/vis/filtering/trajectories/particleFilter-0/add phononL
/vis/filtering/trajectories/particleFilter-0/add BogoliubovQP
```
After this we can go ahead and rerun, which should give us the following image:

<img width="595" height="600" alt="image" src="https://github.com/user-attachments/assets/3105aa6e-ca13-49c5-b64e-233e72981015" />

Now shown in white are tracks of BogoliubovQP objects. To understand what's going on here, le'ts zoom in a bit:
```
/vis/viewer/zoom 16
```
which should give us this: 

<img width="594" height="596" alt="image" src="https://github.com/user-attachments/assets/7b8a89e6-9108-43c9-85e9-db2d35ba854e" />


Here, we see QPs, in white, diffusing around a bit and seemingly being impeded by the gray outlines of our resonator and qubit coupler. Since it's hard to get a full sense of what's going on without seeing the phonons (which simultaneously confuse the field of view) and because we're looking at this only in 2D, let's take a look at a QP track's verbose output. Here, I'm inspecting the beginning and end of track ID 25, a `BogoliubovQP` created by a parent with a track ID of 20. We'll look at the whole track:
```
*********************************************************************************************************
* G4Track Information:   Particle = BogoliubovQP,   Track ID = 25,   Parent ID = 20
*********************************************************************************************************

Step#    X(mm)    Y(mm)    Z(mm) KinE(MeV)  dE(MeV) StepLeng TrackLeng  NextVolume ProcName
    0    -2.04     1.45        5  2.29e-10        0        0         0 ResonatorAssembly_0 initStep
    1     -2.1     1.56        5  2.29e-10        0      523       523 ResonatorAssembly_0 qpDiffusion
    2    -2.17      1.5        5  2.29e-10        0      180       703 ResonatorAssembly_0 qpDiffusion
    3    -2.15     1.51        5  2.29e-10        0     18.6       722 ResonatorAssembly_0 qpDiffusion
    4    -2.11     1.53        5  2.29e-10        0     24.7       746 ResonatorAssembly_0 qpDiffusion
    5    -2.13     1.45        5  2.29e-10        0      319  1.07e+03 ResonatorAssembly_0 qpDiffusion
    6    -2.17      1.5        5  2.29e-10        0      151  1.22e+03 ResonatorAssembly_0 qpDiffusion
    7    -2.19      1.5        5  2.29e-10        0     42.4  1.26e+03 ResonatorAssembly_0 qpDiffusion
    8    -2.19     1.49        5  2.29e-10        0   0.0415  1.26e+03 ResonatorAssembly_0 qpDiffusion
    9    -2.19     1.49        5  2.29e-10        0   0.0206  1.26e+03 ResonatorAssembly_0 qpDiffusion
   10    -2.19      1.5        5  2.29e-10        0     0.19  1.26e+03 ResonatorAssembly_0 qpDiffusion
   11    -2.19      1.5        5  2.29e-10        0    0.132  1.26e+03 ResonatorAssembly_0 qpDiffusion
   12    -2.19     1.49        5  2.29e-10        0    0.491  1.26e+03 ResonatorAssembly_0 qpDiffusion
   13    -2.19      1.5        5  2.29e-10        0     0.26  1.26e+03 ResonatorAssembly_0 qpDiffusion
   14    -2.18     1.49        5  2.29e-10        0     1.08  1.26e+03 ResonatorAssembly_0 qpDiffusion
   15    -2.18     1.49        5  2.29e-10        0     5.51  1.27e+03 ResonatorAssembly_0 qpDiffusion
   16    -2.17      1.5        5  2.29e-10        0     7.63  1.27e+03 ResonatorAssembly_0 qpDiffusion
   17    -2.14     1.49        5  2.29e-10        0     17.9  1.29e+03 ResonatorAssembly_0 qpDiffusion
   18    -2.15     1.44        5  2.29e-10        0     69.1  1.36e+03 ResonatorAssembly_0 qpDiffusion
   19    -2.18     1.42        5  2.29e-10        0     22.5  1.38e+03 ResonatorAssembly_0 qpDiffusion
   20    -2.19     1.42        5  2.29e-10        0     4.91  1.39e+03 ResonatorAssembly_0 qpDiffusion
   21    -2.19     1.42        5  2.29e-10        0   0.0127  1.39e+03 ResonatorAssembly_0 qpDiffusion
   22    -2.19     1.41        5  2.29e-10        0   0.0266  1.39e+03 ResonatorAssembly_0 qpDiffusion
   23    -2.19     1.41        5  2.29e-10        0   0.0169  1.39e+03 ResonatorAssembly_0 qpDiffusion
   24    -2.19     1.41        5  2.29e-10        0 0.000852  1.39e+03 ResonatorAssembly_0 qpDiffusion
   25    -2.19     1.41        5  2.29e-10        0  0.00128  1.39e+03 ResonatorAssembly_0 qpDiffusion
   26    -2.19     1.41        5  2.29e-10        0 0.000382  1.39e+03 ResonatorAssembly_0 qpDiffusion
   27    -2.19     1.41        5  2.29e-10        0  0.00099  1.39e+03 ResonatorAssembly_0 qpDiffusion
   28    -2.19     1.41        5  2.29e-10        0  0.00041  1.39e+03 ResonatorAssembly_0 qpDiffusion
   29    -2.19     1.41        5  2.29e-10        0 1.33e-05  1.39e+03 ResonatorAssembly_0 qpDiffusion
   30    -2.19     1.41        5  2.29e-10        0 1.56e-05  1.39e+03 ResonatorAssembly_0 qpDiffusion
   31    -2.19     1.41        5  2.29e-10        0 3.28e-06  1.39e+03 GroundPlane Transportation
   32    -2.49    0.698        5  1.89e-10        0 1.07e+04  1.21e+04 GroundPlane qpRadiatesPhonon
    :----- List of 2ndaries - #SpawnInStep=  1(Rest= 0,Along= 0,Post= 1), #SpawnTotal=  1 ---------------
    :     -2.49     0.698         5  4.03e-11           phononTS
    :----------------------------------------------------------------- EndOf2ndaries Info ---------------
   33    -2.48    0.998        5  1.89e-10        0 5.41e+03  1.75e+04 GroundPlane qpDiffusion
   34    -2.61     1.26        5  1.89e-10        0 3.45e+03   2.1e+04 GroundPlane qpDiffusion
   35    -2.82     1.61        5  1.89e-10        0 6.05e+03  2.71e+04 GroundPlane qpDiffusion
   36    -3.07     1.04        5  1.89e-10        0 4.33e+04  7.03e+04 GroundPlane qpDiffusion
   37    -2.79     1.16        5  1.89e-10        0 2.45e+03  7.28e+04 GroundPlane qpLocalTrapping 
```
Let's talk about what's happening here. First, the first several steps feature a `qpDiffusion` process, where the QP is just moving around diffusively. Step 31 features a "transportation" step, in which the QP moves from the `ResonatorAssembly_0` volume into the `GroundPlane` volume. In Step 32, we see a `qpRadiatesPhonon` process -- this is doing exactly what it sounds like: lowering the energy of the QP and spitting out a phononTS with that deltaE. A few following steps proceed again via `qpDiffusion`, and then in step 37 the QP dies without fanfare via the `qpLocalTrapping` process.

In this process, there are a few things worth noting. First, the quasiparticle only actually moves in two dimensions, and doesn't change Z position at all. While this is hard to see due to precision involved in these printouts, this is actually exact, and is fundamental to QP propagation in G4CMP. This is true _even for_ the steps that involve inelastic scatters that, say, produce phonons that then travel vertically. As a result, `BogoliubovQP` objects _do not conserve momentum in G4CMP_, though they do (largely) conserve energy. Second, it is pretty clear from both the picture and the printout that QP reflections are happening on vertical interfaces within the thin films, which also implies that there are additional vertical boundaries that must be defined for QPs (and phonons) to properly follow the physics needed in these volumes. 

In summary, we've used this example as a way to get our foot in the door regarding geometry construction, and it's told us a few things:
1. We need to create dedicated volumes for the thin films in which QPs and phonons can physically propagate in 2 and 3 dimensions, respectively.
2. A corollary of point 1 is that for phonons (and QPs) to propagate, we need to define _lattices_ for all of these thin film volumes.
3. We need to create appropriate boundaries for our phonons and QPs to be able to do their physics successfully, including both film/substrate and intra-film boundaries.

The rest of Tutorial Example 1 will be dedicated to discussing how we do these three things, and some good rules of thumb when building them in your own geometry.

> [!TIP]
> Homework question: can you write a new macro to generate a single QP at, say, 200μeV energy, activate both the phonon and QP visualization, and map out which vertices/kinks of the QP track correspond to phonon emission and QP death?

> [!TIP]
> Homework question: what may be the motivation for limiting our QP propagation to only a 2-dimensional plane?

> [!TIP]
> Challenge question: in this verbose output you can see that between Step 32, the X and Y locations of the `BogoliubovQP` indicate a displacement of about 1 mm, but the step length column indicates a step length of about 10 meters (!?). This is not a bug. How/why might this be true?

### Volumes in G4CMP-V10

Let's discuss how to define volumes in G4CMP-V10 using an example volume which we will find within the source file `quasiparticle/src/QuasiparticleResonatorAssembly.cc`. We'll look at the center conducting part of the CPW resonator's final curve before the c-shaped coupler to the Xmon qubit. The relevant block of code in the source file is:
```
//------------------------------------------------------
//Curve 3 (conductor)
G4String curve3ConductorName = pName + "_curve3Conductor";
G4String curve3ConductorNameSolid = curve3ConductorName + "_solid";
G4String curve3ConductorNameLog = curve3ConductorName + "_log";
G4Tubs * solid_curve3Conductor =
	new G4Tubs(curve3ConductorNameSolid,dp_resonatorAssemblyCurveCentralRadius
               - dp_tlCouplingConductorDimY/2.0,
               dp_resonatorAssemblyCurveCentralRadius
               + dp_tlCouplingConductorDimY/2.0,dp_curveEmptyDimZ/2.0,
               180.*deg,90.*deg);
G4LogicalVolume * log_curve3Conductor =
    new G4LogicalVolume(solid_curve3Conductor,aluminum_mat,
                        curve3ConductorNameLog);
G4VPhysicalVolume * curve3Conductor =
    new G4PVPlacement(0,G4ThreeVector(0,0,0),log_curve3Conductor,
                      curve3ConductorName,log_curve3Empty,false,0,true);
log_curve3Conductor->SetVisAttributes(aluminum_vis);
fFundamentalVolumeList.push_back(std::tuple<std::string,G4String,G4VPhysicalVolume*>("Aluminum",curve3ConductorName,curve3Conductor));
```
We first look at the solid volume: a `G4Tubs` object defined as a quarter-circular annulus. There are two main points we want to make about this. First, most of the parameters here are defined with a moniker `dp_` in front, indicating by convention that these parameters are defined in `quasiparticle/include/QuasiparticleDetectorParameters.hh`. Use of this parameters file, which contains not only geometrical values but also some volume-by-volume superconductor parameters, is generally a good idea for code organization. Second, the half-thickness is set to `dp_curveEmptyDimZ/2.0`, which, if you look in the detector parameters file, is set to be the same as `dp_groundPlaneDimZ`. This reuse of the same thickness for the ground plane and the resonator underscores an important point about thin-film geometry construction: all thin-film superconductor volumes should take on the same thickness.

Why do we recommend this? The reason gets back to the fact that QPs are only simulated in 2D and cannot currently diffuse in Z: if films vary in Z thickness, a QP may not be able to access superconducting areas if they do not extend to exactly the same Z at which it is diffusing. A couple of example failure modes are diagrammed below.

<img width="1239" height="267" alt="image" src="https://github.com/user-attachments/assets/8dc71869-0af6-4845-ab53-8ab02da9d136" />

To both help us achieve this uniformity in thickness and to improve the accuracy and cleanliness of our code, we also liberally nest volumes within other volumes in our code (as Geant4 encourages). In the code above, the physical volume construction shows that this volume is a daughter of the `log_curve3Empty` volume, which has the same height but just slightly wider radial bounds. In our zoomed-in image, `curve3Empty` is the volume with the gray bounds, and the `curve3Conductor` created here is the thin cyan annulus. We also note that `curve3Empty` itself is a daughter of the larger base aluminum layer of this resonator assembly, and that base aluminum layer is a daughter of the ground plane. 

<img width="733" height="735" alt="image" src="https://github.com/user-attachments/assets/538e1a49-7349-461b-bfa9-57120df8c351" />

Even though this geometry uses reasonably good nesting practices and class-based templates for the various superconducting structures, the piecewise construction of the resonator (as an example) has a couple of disadvantages.
1. Defining lots of different volumes for the various geometries is somewhat tedious, and each volume will need a complete set of boundaries to its neighbors for proper phonon and quasiparticle physics, so the required effort may balloon quickly with lots of sub-volumes.
2. "Invisible" boundaries (those with no physical distinction between the volumes on either side) like that between the ground plane and the base Al layer of the resonator structure are fine for phonon transport, but overuse of these can complicate the interpretation of the quasiparticle transport.

These disadvantages aside, lots of sub-volumes allows for finer tagging of where our physics is happening, so for this tutorial we keep this.

Sometimes, it may be useful to define geometrical volumes using boolean additions or subtractions of basic Geant4 objects. For example, the Xmon cross volumes (both vacuum and conductor) and the c-shaped coupler on the end of the λ/4 resonator are either single or nested `G4UnionSolids`: 

<img width="650" height="650" alt="image" src="https://github.com/user-attachments/assets/4fd70eed-055c-496b-9522-01b0046ece9c" />

These boolean solids should work reasonably well with the quasiparticle and phonon physics in G4CMP, but...
1. ...deep nestings of G4UnionSolids to build complicated structures is not wise on performance grounds, and in my experience has occasionally made visualization choke as well. While functionally a triply- or quadruply-nested G4UnionSolid will work okay, consider using a G4MultiUnion if you're going to be attempting to link more than a couple basic `G4VSolid` objects (`G4Tubs`, `G4Box`, `G4Trd`, etc.). QP and phonon transport in `G4MultiUnion` objects has been lightly tested and anecdotal evidence points to it giving the correct film response, but this also needs further rigorous exploration, so proceed with caution and skepticism.
2. ...due to the current lack of generality of superconducting plane orientation currently available to the `qpDiffusion` process, one will find best results aligning their superconducting thin film's plane with _both_ the global XY plane _and_ the local XY coordinate systems of the constituent base solids (G4Box, G4Tubs, etc.). For most base solid geometries, this is what one might most naturally do anyway -- for example, a `G4Tubs`' local XY plane is the one described by ρ and φ, which is the plane that one would naturally keep coplanar with a film if one is attempting to build a curved structure into a thin film. However, solids like `G4Trd`, which one may use for a taper in a pad, set the local z direction to be in the direction of the taper, which requires a 90 degree rotation to embed them into the plane of a film. This may cause issues with QP propagation, but for now these issues can be temporarily circumvented by embedding these structures into a `G4UnionSolid` whose base element _does_ follow the coplanarity guidance given above. 

> [!TIP]
> Challenge question: It is possible to define complicated geometries with other construction techniques, which may be more efficient than what we do here. Can you think of a way to define portions of this geometry using the G4ExtrudedSolid class?


### Lattices in G4CMP-V10

With the shapes, materials, and positioning of our superconducting volume placed, we now need to ascribe to it a lattice. While most superconducting films don’t end up as single-crystal lattices without concerted effort during fabrication, we nonetheless ascribe to each superconducting volume a `G4LatticePhysical` object. Let’s look at the lines in `QuasiparticleResonatorAssembly.cc` that do this for `curve3Conductor`:

```
//Need to construct lattice...                                                                             
G4LatticePhysical* AlPhysical_curve3Conductor =
	new G4LatticePhysical(AlLogical,dp_polycryElScatMFP_Al,
                          dp_scDelta0_Al,dp_scTeff_Al,dp_scDn_Al,
                          dp_scTauQPTrap_Al);
AlPhysical_curve3Conductor->SetMillerOrientation(1,0,0);
LM->RegisterLattice(curve3Conductor,AlPhysical_curve3Conductor);
```

There are six arguments to this constructor, the last five of which are meant to provide flexibility of the superconducting response as a function of physical location.
1. First argument, value `AlLogical`: this is the logical lattice associated with the physical lattice, and the argument `AlLogical` is defined at the beginning of the `QuasiparticleDetectorConstruction.cc` file. This is a package of information from the aluminum CrystalMaps file. 
2. Second argument, value `dp_polycryElScatMFP_Al`: This is the length scale for energy-independent, elastic phonon scattering off of polycrystalline grains in this thin film volume. 
3. Third argument, value `dp_scDelta0_Al`: This is the zero-temperature gap value of this thin-film volume.
4. Fourth argument, value `dp_scTeff_Al`: This is the effective temperature of this thin-film volume. Together with `dp_scDelta0_Al`, this governs the rates of pairbreaking, phonon radiation by QPs, and QP recombination via the calculations/formalism set out in [this reference](https://journals.aps.org/prb/abstract/10.1103/PhysRevB.14.4854).
5. Fifth argument, value `dp_scDn_Al`: This is the normal-state diffusion constant for electrons in the material — this is combined with an energy dependent term via the formalism in [this reference](https://journals.aps.org/prb/abstract/10.1103/PhysRevB.58.8225) to capture the dependence of a QP’s transport on its energy.
6. Sixth argument, value `dp_scTauQPTrap_Al`: This is the characteristic “local” trapping time of quasiparticles on impurities. The larger this number is, the slower QPs die due to trapping.
7. A hidden (default-valued) seventh argument is present, which will allow fine-tuned control over the diffusion process to compensate for biases in places where the cost-efficient walk-on-spheres algorithm is unable to properly capture the correct physics. We do not use this here, but will comment more on this in the Tutorial Example 2.

You will need to define a unique physical lattice for each superconducting volume. If you desire, you can keep parameters common across all of them by defining (as we do) those parameters in the `QuasiparticleDetectorParameters.hh` file. 

### Boundaries in G4CMP-V10

With our volume and its lattice defined, let's talk about boundaries. We drop down to the last bit of the `curve3Conductor` construction block in the source file, and see
```
//...and set boundaries with the existing empty (parent) AND the previous                                                         
//conductor (sibling volume);                                                                             
G4String curve3Conductor_boundaryName1 = curve3ConductorName + "_AlVac";
G4String curve3Conductor_boundaryName2 = curve3ConductorName + "_VacAl";
G4String curve3Conductor_boundaryName3 = curve3ConductorName + "_AlAl1";
G4String curve3Conductor_boundaryName4 = curve3ConductorName + "_AlAl2";
new G4CMPLogicalBorderSurface(curve3Conductor_boundaryName1, curve3Conductor,
                              curve3Empty,AlVacBoundary);
new G4CMPLogicalBorderSurface(curve3Conductor_boundaryName2, curve3Empty,
                              curve3Conductor,AlVacBoundary);
new G4CMPLogicalBorderSurface(curve3Conductor_boundaryName3, curve3Conductor,
                              shl7Conductor, AlAlBoundary);
new G4CMPLogicalBorderSurface(curve3Conductor_boundaryName4, shl7Conductor,
                             curve3Conductor, AlAlBoundary);
```
We see that in this block we've defined four boundaries for this volume: two establishing the Al/vacuum boundary between `curve3Conductor` and `curve3Empty` (i.e. the "sidewall" boundaries to the CPW center conductor), and two establishing the boundaries between `curve3Conductor` and the straight horizontal line 7 conductor object, `shl7Conductor`. 

How should I think about why there should be two? Critically, because both phonons and QPs can now _traverse_ boundaries, we have both the opportunity and obligation to provide information about how those particles should see or behave when impinging upon boundaries from either side. This is required for the Al/Al and Al/Si boundaries used in this work, and while not strictly required for the Al/Vacuum boundaries (since phonons or QPs can't exist in vacuum and aren't expected to impinge on the boundary from the vacuum side), we include the "extra" volume for symmetry and overcaution (and in case we wanted to, for example, turn all vacuum into something like LHe, where phonons could propagate.)

If you're paying close attention, you may be asking yourself, "wait, but by this logic there should be _six additional_ boundaries created with other volumes: boundaries with the silicon chip, boundaries with the world vacuum above the CPW, and boundaries with the vertical piece in between `curve3Conductor` and the Xmon coupler, right?" And indeed this is correct. The last of those pairs is actually defined when I define the straight vertical line (`svl1Conductor`) just below this in the file. The other two pairs are created one layer up, in the `QuasiparticleDetectorConstruction.cc` file. 

To conclude this tutorial example, let's take one final look at the boundary definitions themselves, specifically looking at AlAlBoundary to demonstrate how to define boundary parameters.

```
fAlAlInterface = new G4CMPSurfaceProperty("AlAlSurf",
                                          0.0, 1.0, 0.0, 0.0,
                                          0.0, 0.0, 0.0, 0.0,
                                          0.0, 0.0);
```
Here we notice two things: first, the phonon absorption parameter (the sixth argument) and the phonon reflection parameter (the seventh argument) are both zero. If this is true, then we expect 100% transmission of phonons through Al-Al interfaces. Second, there are two extra parameters governing user-tunable interfacial properties of `BogoliubovQP` particles: an absorption and a reflection parameter. Since both of these are _also_ zero, it implies that we get full QP transmission through this interface, as long as the QP has sufficient energy to be above the gap of the post-step volume. (If not, then it will reflect with 100% probability). 

We refer readers who have forgotten about how these parameters are applied to achieve absorption, reflection, and transmission with various probabilities back to our favorite lines of code in all of G4CMP: `G4CMPBoundaryUtils::ApplyBoundaryAction()` (as also shown in the original RISQTutorial).


### Extra topics: Gap Engineering, Bilayers, and "How Small Can I Make Things?"

While we will not demonstrate geometries with varying gaps in this demo, it’s worth commenting on some of the slightly more complicated geometrical cases for which we ultimately intend to hone this new G4CMP capability.

First, we consider a low-gap region for quasiparticle trapping connected to a much larger higher-gap collection fin, as in the design of the SQUAT-type device. These connections usually take the form of a small region in which the materials of different gaps overlap in XY, forming a stack in Z. The physics of QP diffusion then gives QPs in this overlap region several “attempts” to trap, i.e. fall below the gap energy of the collection fin and be unable to return. Since our purely horizontal QP diffusion cannot by definition successfully model this effect, it is useful to point out another knob which may emulate it. While G4CMP-V10’s BogoliubovQP physics lists natively manage the ability or inability of a QP at a given energy to enter into a nearby region of higher or lower gap, the reflection parameter at the boundary results in an additional artificially tunable reflection at this purely vertical boundary. If one tunes this parameter (perhaps with another intermediate volume defined to assist with QPs remaining “close” to the trap), this may provide enough parameters to successfully model a trap with an overlap region. Ultimately, a dedicated validation of such a system and an appropriate map of a strategy onto this architecture is needed.

Another example that is also tricky given the current architecture are bilayers. Since QPs cannot diffuse vertically, much of the real physics present in a bilayer may not be accessible with the current simulation architecture. However, in some cases, where bilayers are in highly localized regions in XY, it may be possible to roughly model these as side-by-side volumes and capture some of the physics. This claim, too, is unexplored and needs further demonstration.

Lastly, given that the superconducting device field often uses small device/sensor geometries, it's useful to comment a bit on "how small is too small" to expect realistic microphysical evolution of phonons and QPs. First, Geant4 has tolerances on the scale of a picometer, so certainly devices need to be much larger than this scale for accurate modeling of anything. However, the `qpDiffusion` process also has longer length scales embedded that help it achieve computational efficiency, and those are currently set such that the shortest length scale (e.g. trace width, junction width, hole radii, etc.) we recommend is of order 100 nm. If your devices have significantly finer features than this, we may be able to still model them, but shoot an email to linehan3@fnal.gov to ask the best way to do this.


## Tutorial Example 2: Scanning Energy Depositions in a Planar Resonator

We now present a tutorial based on the same geometry that is intended to showcase some of the power of G4CMP-V10's new QP tracking. In this, we will study the time evolution of the quasiparticle density in the top left superconducting resonator on the chip, for energy depositions at two different points: one in the ground plane (Point 1) and one directly within the coplanar waveguide's inner conductor (Point 2).

<img width="690" height="686" alt="image" src="https://github.com/user-attachments/assets/df48c971-5ca2-44c5-81cb-a79a00425dd6" />


We'll use a new macro this time, `quasiparticle_resonator_targeted.mac`. In this macro, we will simulate high-energy (Debye-energy) phonons in the superconductor as a way of mimicking a fraction of an energy deposition from, say, an optical calibration source, and will store quasiparticle step information for analysis. As we proceed, we'll split this work up into three parts: running the simulation, analyzing the data, and tuning parameters. 


### Running the Simulation and Generating Quasiparticle Step Info

Let's go ahead and just run our new macro out of the box:
```
./g4cmpQuasiparticle
/control/execute ../quasiparticle/G4Macros/quasiparticle_resonator_targeted.mac
```
It's first set up to launch three events, each with a single 36.9 meV phonon (Al Debye energy, a la [this reference](https://journals.aps.org/prb/abstract/10.1103/PhysRevB.61.11807)) in the ground plane at Point 1 in the above diagram. This should run for about 2 minutes, and should produce a visualization that looks like the left image below:


<img width="1276" height="628" alt="image" src="https://github.com/user-attachments/assets/07c4e93b-b4a2-408f-a2c9-7a98e7886dc9" />


You may also observe a few exceptions thrown, including but not limited to messages such as
```
-------- WWWW ------- G4Exception-START -------- WWWW -------
*** G4Exception : QPDiffusion025
      issued by : G4CMPQPDiffusion::GetMeanFreePath
When calculating safety from a boundary, the2DSafety is below one picometer, and we find a triple junction. Safety: 7.24732e-10. If small enough this may cause issues with surface norm finding. Killing track.
*** This is just a warning message. ***
-------- WWWW -------- G4Exception-END --------- WWWW -------
```
These exceptions kill individual quasiparticle tracks that have been produced and which have wandered into locations that are challenging to properly handle using the diffusion code embedded into a Geant4-esque Monte Carlo with as complicated of a planar geometry as we're using. We can also estimate roughly the level at which these errors artificially kill tracks: on average, during the simulation we just did, we deposit about 111 meV of energy into the film, of which about 57% is expected to be converted into QPs. Divide that energy into the Al gap energy (~178 μeV), and we get about 250 QPs created. While running, I got about 1 exception, which means that about 0.4% of our QPs were artificially killed. This is higher than what we ideally want, but is also heavily geometry and parameter-dependent, and these errors will be a future focus of refining the diffusion code.

The main output from this is a file called `QuasiparticleStepInformationFile.txt`. We'll open that in a sec, but for now let's rename it and rerun, since our next sim will actually take about 10-20 minutes to run (though this may vary depending on your computing hardware), and it will overwrite this output file if we keep it the same name. Note that this next file will nominally use about 3GB of disk space. If you don't have that, we recommend cleaning up some disk space if you want to continue. After exiting G4CMP's interactive session, we can run
```
mv QuasiparticleStepInformationFile.txt QuasiparticleStepInformationFile_Point1_3evts.txt
```
so that we name the file with some useful labels that we can use to remember what went into it. Once we do this, let's go into our macro and edit it to simulate interactions at Point 2, by changing the position definition lines to:
```
#/gps/pos/centre -2.58 1.221 5.0001 mm # Point 1, in film
/gps/pos/centre -1.85 1.221 5.0001 mm # Point 2, in film
```
Now rerun the simulation with this amended interaction point, which is dumping our Debye-energy phonon into the central conductor of our CPW resonator at Point 2. 
```
./g4cmpQuasiparticle
/control/execute ../quasiparticle/G4Macros/quasiparticle_resonator_targeted.mac
```
This process should take much longer, about 10-20 minutes, and will give an output as shown on the right half of the above image. (In the meantime, we'll move on and talk about analysis of the output file we've already collected.) Once this finishes, let's go ahead and rename this as well:
```
mv QuasiparticleStepInformationFile.txt QuasiparticleStepInformationFile_Point2_3evts.txt
```

> [!TIP]
> Homework question: why does it make sense that this simulation takes longer? Given the walk-on-spheres technique, can you estimate a naive scaling for how much longer this simulation should take than the simulation for Point 2? Does it agree with the observed difference in execution time?


### Analyzing the Simulation Output

In this analysis, our goal will be to plot the time evolution of the quasiparticle density within two regions of our CPW. This kind of analysis could be relevant for understanding the response of CPW resonators' response to QP poisoning at different points along their length, which may vary due to the spatially varying current and voltage along the resonator. For simplicity, we'll look specifically at the quasiparticle population in two half-circles at the ends of the meander: half-circle 6 and half-circle 1:

<img width="600" height="600" alt="image" src="https://github.com/user-attachments/assets/7633dab8-2ec8-4c23-80ab-680ae2facdef" />

To begin analyzing our output, we first need to understand the structure of our output. Unlike the original RISQ 2024 tutorial example, we're no longer thinking primarily about "hits" -- here, the analysis we're going to do will be entirely based on track stepping information. (Notably, hits may still be used and defined as they were in the 2024 RISQ tutorial, which opens up space for analysis with both stepping and hits. However, we focus primarily on the stepping analysis here.) 

We've provided an analysis macro that can read the output of the simulations we've just run, but it's useful to first look at what we're _trying_ to save. This takes us to the source file `quasiparticle/src/QuasiparticleSteppingAction.cc`, which has two critical blocks of code. The first is the location where we determine what to do during every step (i.e. a user-defined action to do per-step):

```
//Alternative constructor                                                                      
void QuasiparticleSteppingAction::UserSteppingAction(const G4Step* step) {
  //First up: do generic exporting of step information (no cuts made here)                     
  ExportStepInformation(step);
  return;
}
```
Here, during every step, we simply just run a function that exports step information to file. This is kept as a separate function so that in case we want to do other non-export-related calculations within the stepping action, we can also put those in the `UserSteppingAction` function without mixing them into the exporting functions. Within the `ExportStepInformation()` function, defined below this one, the critical block of code is
```
//Only fill stepping output with QP information                
if( particleName.find("BogoliubovQP") != std::string::npos ){

	//Fill the output file with the step info                                
    fOutputFile << runNo << " " << eventNo << " " << trackNo << " "
                << particleName << " " << std::setprecision(14) << preStepX_mm
                << " " << preStepY_mm << " " << preStepZ_mm << " "
                << preStepT_ns << " " << preStepEnergy_eV << " "
                << preStepKinEnergy_eV << " " << postStepX_mm << " "
                << postStepY_mm << " " << postStepZ_mm << " " << postStepT_ns
                << " " << postStepEnergy_eV << " " << postStepKinEnergy_eV
                << " " << nReflections << " " << stepProcess << " "
                << preStepVolume << " " << postStepVolume << std::endl;
}
```
In this block (which runs after gathering all of the step info that is exported here) we write information to our stepping output file if the particle taking the step is a `BogoliubovQP`. For that QP, we write pre- and post-step information including X, Y, Z, T, E, KE, and volume name variables, as well as the total number of reflections at this step and the process that determines the step. Notably, we do this for _all_ QP steps, without discrimination. As you may have already noticed looking at your output files from the last section, this can produce reasonably large output files even for small numbers of events. For now, we'll accept this, with the recognition that we can trim these files using additional conditionals in the above block of code to request that the simulation only saves the steps we care about.

> [!CAUTION]
> Running far more events in our `quasiparticle_resonator_targeted.mac` than what we've already set up without further conditionals on what is saved may quickly fill up your disk. For Point 1, expect about 30 MB per event, and for Point 2, expect about 1 GB per event. Proceed with caution.

With this information, let's go ahead and run our analysis macro using ROOT. We'll start up an interactive ROOT session and run
```
root -l
.L ../quasiparticle/AnalysisTools/quasiparticle_analysis.cc
run_quasiparticle_analysis("/path/to/QuasiparticleStepInformationFile_Point1_3evts.txt","/path/to/QuasiparticleStepInformationFile_Point2_3evts.txt")
```
This will take some time to run -- it reads the stepping output files, which are between a few hundred MB and a few GB, and builds simple event structures out of them. It then analyzes those events, plotting basic information like quasiparticle creation/destruction times and locations, all step locations, and finally the time-averaged quasiparticle occupations of our resonator structures. We'll first explore some basic cross-check plots, which is an important step in any analysis to make sure that our desired results make sense.

Let's go ahead and open up a TBrowser, where we can start poking around the plots that are made:
```
TBrowser a
```
Within our output file, we have two directories, one for each analyzed file, so that we can compare "apples to apples" plots between the two conditions. As a first cross check, let's plot the starting locations of quasiparticles produced in our simulation with phonons spawned at Point 1: for this we'll open up the `qp_startXY`, `qp_startXZ`, and `qpStartYZ` histograms:

<img width="1229" height="417" alt="image" src="https://github.com/user-attachments/assets/c210a220-6512-4c29-ac0a-ca8b53de1741" />

_Do these make sense?_ Given the physics of the initial, high-energy downconversion cascade, we might expect a lot of quasiparticles produced at the point very near where our original Debye-energy phonons were spawned. We do see this in XY (see inset), but we also see lots of quasiparticle starting points far (100's of μm to mm) from this initial point. Why might this be? This could reasonably be due to a handful of effects:
1. As QPs radiate off phonons, those phonons may leave the superconducting film, bounce around the chip to different, more distant locations in the chip and re-break pairs in the film there.
2. QPs that recombine in the vicinity of the initial Debye-energy phonon may produce near 2Δ phonons that can then go pairbreak elsewhere as well.

So okay, this seems sensible. For XZ and YZ, we do see that all of our QP creation points occur in the thin film on the top of the 400μm chip, and there are no other QP steps taken elsewhere (as expected).

Let's move on to  cross-checking our plots of _all_ QP steps. Here, now focusing entirely on XY because our XZ and YZ plots all (thankfully) look the same, we find the following plot for Point 1.

<img width="574" height="576" alt="image" src="https://github.com/user-attachments/assets/3905dad3-68e1-47af-89c8-aa3712b0a883" />

_Does this make sense?_ Once we create our QPs, they are free to diffuse according to the mechanics of the `qpDiffusion` G4CMP process. Coarseley, we do see that there are lots of steps that are occurring in the top left of the chip where we are launching our Debye phonons, which makes sense given that most of our QPs are being generated in this region. However, there are a handful of other features that are worth pointing out.
* Some features seem a bit artificial, such as the rectangular "doorway" surrounding the resonator. This "doorway" is as a result of the fact that I've defined a base layer of aluminum into which all of the resonator features are embedded as daughter volumes. QPs impinging upon this base layer will end their step and transport across the boundary here if they encounter it, implying an artificial overdensity of points here.
* Overdensities of steps in between the meanders of the resonator: here, it seems like the quasiparticle is spending more steps inside the regions bounded by the resonator turns, which seems aphysical. Why should the quasiparticles spend longer in this region than anywhere else in the ground plane? A key resolution to this question is that _steps undergone are not necessarily proportional to time elapsed_. This is a critical point to digest while using this new quasiparticle tracking bit of the code: diffusion steps are largely meaningless without a clear ability to monitor the _time_ over which those steps are occurring. Here, the overdensity of steps in the meander is occurring because the diffusion algorithm is more constrained by geometry, which forces it to take shorter steps. Out in the bulk ground plane, QPs can take single steps over which much more physical time elapses, thanks to the lack of nearby boundaries.
* Overdensities of steps near the corners of the chip: again, we're in a place where the stepping algorithm is relatively constrained by boundaries. If a QP is in a corner, then it's constrained in multiple dimensions, and will take shorter steps.

> [!IMPORTANT]
> A critical takeaway: since quasiparticle diffusion steps are geometry-driven (Walk-on-Spheres technique), step information is mostly meaningless unless coupled with the time over which a step occurs as well.

To get a more physically accurate picture of where QPs diffuse, we can look at a physical observable, something driven by a process other than diffusion. We'll look at the _endpoints_ of the QP tracks, which are dictated by the times at which QPs either trap or recombine, but which diffuse in XY from their creation to this time. Since we're now limiting ourselves to the small-ish, O(1000) number of QPs, we'll actually coarse-bin our histograms a bit to help display this. On the left below we re-show `h_qpAllStepsXY` for Point 1, and on the right we show `h_qpLastStepPostXY` for Point 1, with an additional re-binning of 4 on both axes.

<img width="1266" height="616" alt="image" src="https://github.com/user-attachments/assets/ae102177-d8a6-4060-8e1e-1f81b453524e" />

We now see that even though there is a massive nonuniformity in the locations of all steps (left), the locations at which the QPs end up dying look a bit more uniformly distributed around the creation point, while still being constrained by the CPW structures, as we expect.

One last thing we can do for a cross check is to look at the lifetimes of the quasiparticles, by just taking their final step post-step time and subtracting it from their initial step pre-step time. From our 472 quasiparticles created in this event, we get the following distribution:

<img width="629" height="573" alt="image" src="https://github.com/user-attachments/assets/0d12bc97-2173-44cc-a5fc-38537d04c123" />

From the mean in the stat block on the top right, we can get a rough sense of the QP lifetime: around 158 μs. Does _this_ make sense? There are two contributors that will non-artificially kill a quasiparticle: trapping and recombination. To understand what parameters we're using for this, let's take a look in our favorite parameters file: `quasiparticle/include/QuasiparticleDetectorParameters.hh`. At the top of this file we have:

```
constexpr double dp_polycryElScatMFP_Al = 10 * CLHEP::nm;
constexpr double dp_scDelta0_Al = 0.000176 * CLHEP::eV;
constexpr double dp_scTeff_Al = 0.2 * CLHEP::kelvin;
constexpr double dp_scDn_Al = 6 * CLHEP::um*CLHEP::um / CLHEP::ns;
constexpr double dp_scTauQPTrap_Al = 0.2 * CLHEP::ms;
```
which says that our local trapping time is set to 200 μs, and that our effective temperature, which governs recombination, is set to 200 mK. This effective temperature produces a QP recombination lifetime in Al (from [this reference](https://journals.aps.org/prb/abstract/10.1103/PhysRevB.14.4854)) of around 1 ms at energies just above the gap. With these two separate decay timescales, the joint decay time should be 1/(1/200μs + 1/1ms)~160 μs. This is indeed what we see.


> [!TIP]
> Homework problem: What is a plot you can make that will allow you to check if there are any additional biases that may be contributing to an artificially deflated quasiparticle lifetime? Are there any already in the ROOT file that you might be able to study to ascertain whether there's a bias?

> [!TIP]
> Homework problem: The reader is encouraged to run through the same set of cross-checks for the analogous Point 2 plots.

Ok, so now let's look at the plots we stated in our goal: the time-averaged QP occupation of various resonator structures. The three plots of interest are the three `h_fullResQPTimesXXXXX` histograms, which show the time-averaged QP presence in:
* Any chunk of the resonator central conductor, from the transmission line coupler to the c-shaped qubit coupler
* Just half-circle 1
* Just half-circle 6

For Point 1, these three plots are shown below (noting that we zoomed in on the x-axis in the TBrowser for clarity):

<img width="1202" height="830" alt="image" src="https://github.com/user-attachments/assets/07c90686-e36b-4a5f-a5ed-ebd2eb7185cf" />

From these we can gain a bit of direct insight into QP dynamics directly within the central conductor. First, on average, for this set of simulation parameters, there aren't that many QPs that are created and exist anywhere within the central conductor in response to our initial phonons: the left plot shows that on average, only either 1, 2, or 3 QPs is really present anywhere. Moreover, right two plots, we find them nearly empty. These are consistent with the simulation rendering above -- only very short regions of QP tracks can be seen encroaching into the half-circle 1 or half-circle 6 volumes, suggesting that these QPs either diffuse back out of these regions or recombine/trap quickly after entering. As a result, we get a "fractional" time-averaged QP population: for those time bins, a single QP only occupies that bin for a fraction of the length of that time bin.

Let's now compare this to our Point 2 simulations, which are shown below:

<img width="1151" height="797" alt="image" src="https://github.com/user-attachments/assets/13dfd1df-704e-498c-bce4-4378a0a059c7" />

Here, we have a very different story. The left plot, again showing the QP presence in the entire CPW, shows a distinct quasi-exponential falloff in QP occupancy (with our aforementioned decay time) until reaching the few-QP regime, at which point statistical fluctuations determine the shape of the plot. The QP occupancy in half-circle 1 (center plot) is now nonzero, and notably shows a QP peaked not at zero but at some time roughly 0.3 ms after the Debye phonon injection. This timescale most likely comes about due to diffusion of initially-produced quasiparticles, but let's confirm this intuition to see if it makes sense. The rough distance that this QP needs to travel to get to half-circle 1 is, based on the geometry of the meander, about 2.4 mm. The diffusion constant for QPs is given by this expression:

<img width="400" height="142" alt="image" src="https://github.com/user-attachments/assets/8430e220-d72a-49cb-8411-fac6e59a384b" />

Where D_n is the normal-state diffusion coefficient just above the transition temperature. In G4CMP, our value for this is about 6 um^2/ns. The dependence on the QP energy means that for most QPs that have cooled, the diffusion coefficient will be a factor of a few lower than this D_n out front, so we'll guess for now that the diffusion coefficient is about 2 um^2/ns. Then, we can calculate what average Δt we may expect for a given diffused Δx using (Δx)^2/2D. For our numbers here, using Δx=2400 um and D=2 um^2/ns, we find an expected time to diffuse of about 1.1 ms. For a back of the envelope, this is not bad. However, we also recognize that additional pairbreaking may occur in regions of the CPW far from the initial injection point due to phonons released in the initial cascade, which may help explain why QPs are present in this distant location a bit earlier than our estimate.

The final plot on the right, showing the QP occupancy of half-circle 6, is much more populated with QPs, but also displays a pattern of QPs "hopping in and out" of that section at late times, once many of the initial QPs have decayed away. Overall, this scenario shows a significantly different outcome compared to the Debye phonons launched into the ground plane (Point 1): here, we don't have to "get lucky" and have a QP produced in the CPW from a phonon that happens to enter it after being produced far away.

### Parameter Tuning

Carefully considering what parameters to tune for your application is tricky business. While we won't exhaustively walk you through a tutorial on how to tune these, we can give a few pointers on parameters that are likely to be most dominant for looking at typical devices that monitor quasiparticle populations. We again return to our favorite set of parameters in the `QuasiparticleDetectorParameters.hh` file:
```
constexpr double dp_polycryElScatMFP_Al = 10 * CLHEP::nm;
constexpr double dp_scDelta0_Al = 0.000176 * CLHEP::eV;
constexpr double dp_scTeff_Al = 0.2 * CLHEP::kelvin;
constexpr double dp_scDn_Al = 6 * CLHEP::um*CLHEP::um / CLHEP::ns;
constexpr double dp_scTauQPTrap_Al = 0.2 * CLHEP::ms;
```
Arguably, the three most important parameters here are the zero-temperature gap, `dp_scDelta0_Al`, the effective temperature of the superconductor, `dp_scTeff_Al`, and the trapping lifetime, `dp_scTauQPTrap_Al`. Since the zero-temperature gap should be fixed in principle, this largely is a "set and forget" parameter. The other two are less obvious _a priori_, and are parameters you can tune to match simulations to your data.
1. `dp_scTeff_Al`: This effective temperature dictates the recombination, phonon radiation, and pairbreaking lifetimes via the formalism in the [Kaplan paper](https://journals.aps.org/prb/abstract/10.1103/PhysRevB.14.4854). While phonon radiation and pairbreaking lifetimes do vary with this, the recombination lifetime is _strongly_ dependent on this, and diverges at low values of (T_eff/T_c). As a result, if you do not have other mechanisms for quasiparticles to die (i.e. local trapping), then setting T_eff below about 15% of T_c will make your code run for a VERY long time. We'll note that when QPs die via this mechanism, they release a near-2Δ phonon with a 50% probability, to conserve energy globally. This permits "QP recycling," in which subsequent cycles of pairbreaking, recombination, and re-pairbreaking keep a QP's influence in the chip around for longer than the time between its initial creation and (initial) death.
3. `dp_scTauQPTrap_Al`: This local trapping lifetime is set directly, and will kill quasiparticles without fanfare, i.e. without any phonon emission.

> [!CAUTION]
> We suggest you explicitly acknowledge what loss channels you have baked into your simulation before running it. Do you have phonon absorption set to a nonzero number at any thermal mounts? Are you using a finite (i.e. non-infinite) value of `dp_scTauQPTrap_Al`? If the answer is "no" to all of these questions, **you may find yourself in a situation where your code never stops running.** While `phononBounces` and `qpBounces` may still kill your phonons and QPs, respectively, phonon recycling "resets" this number: if a QP recombines into a phonon which produces 2 more QPs, both of those QPs start with a zero bounce count.

One last thing to mention are some subtleties with the diffusion code that advanced users may notice:
1. There are obviously still some bugs needing working-out, which are rare, and become rarer for simpler geometries. Stay tuned as we fix these over the next several months.
2. The walk-on-spheres algorithm itself is actually not fully bias-free, and in particular has a very challenging time "looking around corners." This adds a bit of additional bias if you are trying to model systems where a broad area funnels into a thinner area, such as the geometry shown below. Unfortunately, these kinds of systems are commonplace in the QIS environment. (Ex. the locations where Josephson Junctions meet the superconducting pads, and the locations where . Fortunately, there _is_ a way to mitigate this bias, though this technique is still under investigation and is a bit beyond this tutorial. If you're curious about applying this, please talk with Ryan.

<img width="600" height="420" alt="image" src="https://github.com/user-attachments/assets/b13dfcad-b86c-4b8b-857e-173c60448acf" />

And with that, we'll conclude with a handful of homework problems for particularly motivated readers.

> [!TIP]
> Homework problem: repeat the analysis in the above section, but after having turned off recombination and set the trapping timescale to 1 ms. What do you observe in terms of execution time? Output filesize? In-resonator QP occupations?

> [!TIP]
> Homework question: can you think of a way to meaningfully cut down stepping output filesize while still getting results similar to what we've seen here?

> [!TIP]
> Challenge question: for QPs produced in the middle of the CPW resonator, what's a technique that you can add to the existing script to test the accuracy of the diffusion through the CPW? 

