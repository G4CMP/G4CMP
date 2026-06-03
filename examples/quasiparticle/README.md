# Tutorial: Quasiparticle Propagation in G4CMP

This example is a demonstration of several features and physics lists that are new to G4CMP as of November 2025. Ryan Linehan, linehan3@fnal.gov made this, so please email him with questions/compliments/complaints.

## Preliminaries

The preliminary requirements for running this example are the same as those needed for other examples such as the RISQTutorial example from RISQ 2024. For completeness, we will repeat these instructions.

### Installing Geant4 and G4CMP
We'll start with a reminder that in order to run this example, you'll need to install ROOT and both the geant4 and G4CMP packages. On my machine, each of these has three directories associated with its build: a source directory `XXXXX`, a build directory `XXXXX-build`, and an install directory `XXXXX-install`. On my machine, the base name (`XXXXX`) for my geant4 build is `geant4.10.07.p04`, and the base name for the G4CMP build is `G4CMP_quasiparticle`. NB: the last steps in the process of each of these installations should be to run `make` and `make install` while in the `XXXXX-build` directory, for both geant4 and G4CMP


> [!IMPORTANT]
> To streamline your ability to prep for this tutorial, we recommend installing Geant4 with the cmake flags `-DGEANT4_INSTALL_DATA=ON` and `-DGEANT4_USE_OPENGL_X11=ON`, and to build with C++14. In particular, the OpenGL flag will enable visualization, which we will frequently use. However, if you can successfully run other visualizers like DAWN, those are also perfectly fine.

> [!IMPORTANT]
> You need to update your local G4CMP copy to at least G4CMP-v10 for this tutorial, but we recommend pulling the most current version of the `develop` branch and reinstalling. 

### Setting up environment
Assuming you've built these directories and you're opening up a new terminal, you'll need to source the environmental setup scripts for these:
```
source /path/to/geant4.10.07.p04-install/bin/geant4.sh
source /path/to/G4CMP-install/share/G4CMP/g4cmp_env.sh
```
Now we can make our example. Copy this tutorial's source directory into a new directory -- I like to copy it outside of the whole G4CMP source directory just to avoid confusion and remember that this is its own executable that needs to be made. Moreover, make build and install directories to accompany it:
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
cmake -DCMAKE_INSTALL_PREFIX=/path/to/quasiparticle-install -DCMAKE_CXX_STANDARD=14 ../quasiparticle/
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
 	* This is a doozy of a function. It uses an efficient MC approach to diffusion in a generalized geometry called Walk-on-Spheres to do diffusion steps of QPs in thin films. Currently only implemented in 2D, and moreover only currently implemented in XY specifically.
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

> [!TIP]
> For those interested in a look under the hood, you can take a look at which processes are implemented for which particles in G4CMPPhysics.cc

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


### When should you use these 














In the G4 macro used to run events, these processes can be turned on and off (mostly) at your discretion. An example code block looks like so:


```
#-------------------------------------
# Turn on/off new processes
#/process/inactivate phononScattering
#/process/inactivate phononDownconversion
/process/inactivate phononPolycrystalElasticScattering
#/process/inactivate qpRecombination
#/process/inactivate qpRadiatesPhonon
#/process/inactivate scPairBreaking
#/process/inactivate qpDiffusion #If QPs can exist, diffusion needs to be active
#/process/inactivate qpLocalTrapping
/process/inactivate qpDiffusionTimeStepper
```

In this example, phononScattering and phononDownconversion are the ``old,'' i.e. already-existing phonon processes, and the rest are part of the TrackedFilmResponse branch. 

> [!CAUTION]
> Most combinations of these processes being on/off work, but QP transport will be UNDEFINED if the qpDiffusion process is turned off. In other words, if you want to do ANY quasiparticle dynamics (say, by looking at "just" phonon pairbreaking), QP diffusion will need to be on. If you want to look at "just" QP phonon radiation, QP diffusion will need to be on as well, etc. The only scenario in which qpDiffusion can be inactive is one in which no tracked QPs are expected to be produced in the simulation.

Final note: for completeness, `qpDiffusionTimeStepper` is a "second" way of doing diffusion, but also requires the qpDiffusion process to be on. It's more of a diagnostic tool and actively slows the code down relative to just using `qpDiffusion`, so most people shouldn't need or want to use it. If you're considering this, may be good to check in with Ryan (linehan3@fnal.gov) to see if it's something you're actually wanting to do.




