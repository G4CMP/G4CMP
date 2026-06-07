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
source /path/to/G4CMP_quasiparticle-install/share/G4CMP/g4cmp_env.sh
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


### When should you tailor your simulation to use these processes?

An important question to ask about building or running _any_ simulation is, "what advantage do I get from running it?" On one hand, undermodeling a system may make it challenging to fit important features of experimental spectra or time series. On the other hand, it is also possible to overmodel a system by providing more parameters than needed, sweeping over an excessively large parameter space, overfitting, and wasting computational resources in doing so. So how do you strike a balance? 

Rather than being prescriptive about what applications can make best use of this new addition, we propose flavors of problem that this new version can now model better. These include devices whose signal or performance is reasonably dependent on:
* The locations and energy distribution of quasiparticles produced in an interaction
* The transport of quasiparticles across regions of varying superconducting gap Δ.
* Phonon recycling (i.e. QP recombination into phonons that rebreak Cooper pairs)
Some examples of these include superconducting qubit devices, where diffusion plays a role in governing the population of QPs that can make it to the junction for tunneling, as well as resonators, where different densities of QPs at different points on the resonator may produce variable signals (CITE). We'll look at an example of the latter in our Tutorial Example 2.

However, if your application does not need to model these effects, a more limited superconducting response is still handled by the KaplanQP class within G4CMP, and executes significantly faster. This works relatively well for spatially limited devices, but currently ignores QP recombination.

## Tutorial Example 1: Geometry Construction

The goal of this section is to introduce advanced users to new elements of geometry construction that are needed for adopting the new features of G4CMP-V10. We're going to start in the approximately the same place as the original RISQ Tutorial example: by inspecting a moderately complicated superconducting qubit geometry, once again based on the "candlestick" Xmon qubits designed by the McDermott Group at UW-Madison. The following features are on this chip:
* Silicon chip substrate
* Aluminum ground plane
* Aluminum CPW transmission line and wirebond pads
* Aluminum CPW quarter-wave resonators (x6)
* Aluminum Xmon qubits (no junction leads included)
* Copper mounts for thermalization
To see these features, we'll run our first macro, `quasiparticle_geometry_vis.mac`, from our `quasiparticle-build` directory. First, we'll navigate there and boot up G4CMP:
```
cd /path/to/quasiparticle-build
./g4cmpQuasiparticle
```
This will start an interactive session, and now we can run our macro. If you've followed the installation instructions above and kept `quasiparticle` and `quasiparticle-build` separate at the same level of the filetree, then you can just run
```
/control/execute ../quasiparticle/G4Macros/quasiparticle_geometry_vis.mac
```
which should give you the following result, which we'll reference in the several following sections on geometry construction:

<img width="591" height="597" alt="image" src="https://github.com/user-attachments/assets/31d1928d-6ab5-4852-9afc-9ba29648e8b8" />

The overall structure of the geometry construction, created in `quasiparticle/src/QuasiparticleDetectorConstruction.cc`, is very similar to that made for the 2024 RISQTutorial, and we refer readers to that tutorial for more thorough guidance on that overall structure. However, it is worth reminding readers that:
* The silicon chip and superconducting ground plane are siblings in the geometry heirarchy
* The transmission line and resonators are defined in dedicated classes...
* ...and are daughters of the ground plane but are siblings to each other in the geometry heirarchy.
* The dedicated class objects (transmission line, resonators) pass volume information up to the main detector construction file when defining boundaries between these objects' subvolumes and the immediate daughters of the world (like the silicon chip).

### Geometry Heirarchy and Boundary Requirements in G4CMP-V10

The fundamental improvements brought by G4CMP-V10 are that phonons can transmit between volumes and create trackable QPs in those volumes (if applicable). This implies that full use of these features requires a tweak to how volumes and boundaries are defined in G4CMP.  

First, since phonons now may pairbreak over a characteristic mean free path in a superconducting material, we need to properly define the thicknesses of the superconducting films directly in the geometry, as opposed to via the `filmThickness` parameter of the KaplanQP class. In prior examples such as the 2024 RISQTutorial, the detector construction was _defined_ with a thickness, but this thickness effectively did nothing: all microphysics in the superconducting plane was handled either by KaplanQP or through a simple absorption. Now, thickness directly impacts the number of quasiparticles produced. This thickness is defined as `dp_groundPlaneDimZ` in `quasiparticle/include/QuasiparticleDetectorParameters.hh`. 

DESCRIBE QP TRANSPORT HERE AND PUNT DISCUSSION OF BILAYERS TO LATER

Technically, one can establish films of varying thickness, but for reasons related to quasiparticle transport (described below), we currently recommend defining all superconducting thin films with the same thickness.

> [!TIP]
> Use of the `QuasiparticleDetectorParameters.hh` file to co-locate all of your geometry parameters for organization is good practice! Note also that hardcoding the fewest number of parameters that you can while maintaining the flexibility you need is also good practice. The thicknesses of all of the superconducting structures in this example are all given different names for clarity and readability, but all use the singular value of 90nm given to `dp_groundPlaneDimZ`.

In creating this geometry, we liberally make use of parent-daughter definitions to improve accuracy and cleanliness of our geometry code. To see an example of this, let's run the following command to zoom in on a particular part of the geometry, from where we left off in our interactive session:
```
/vis/viewer/zoom 16
```
This should bring you to the following visualization (without the arrows/text)
<img width="650" height="650" alt="image" src="https://github.com/user-attachments/assets/f773920b-14c7-41cc-ae5d-e92fc93feae3" />

Even within our resonator assembly, we have a triply-nested heirarchy for the CPW resonator structure. The top level mother is the Al base layer in which all of our structures within a single resonator live. The half-circle of vacuum defined for the CPW "trenches" in the sixth turn of the meander is a daughter of this base layer, and the half-circle of Al defined as the central conductor of the CPW is a daughter of this vacuum volume. Especially for coplanar structures like these resonators and qubits, we recommend following this nesting technique. 

Even though this geometry uses reasonably good nesting practices and class-based templates for the various superconducting structures, the piecewise construction of the resonator (as an example) has a couple of disadvantages.
1. Defining lots of different volumes for the various geometries is somewhat tedious, and each volume will need a complete set of boundaries to its neighbors for proper phonon and quasiparticle physics, so the required effort may balloon quickly with lots of sub-volumes.
2. "Invisible" boundaries (those with no physical distinction between the volumes on either side) like that between the ground plane and the base Al layer of the resonator structure are fine for phonon transport, but overuse of these can complicate the interpretation of the quasiparticle transport.

These disadvantages aside, lots of sub-volumes allows for finer tagging of where our physics is happening, so for this tutorial we keep this.

Sometimes, it may be useful to define geometrical volumes using boolean additions or subtractions of basic Geant4 objects. For example, the Xmon cross volumes (both vacuum and conductor) and the c-shaped coupler on the end of the λ/4 resonator are either single or nested `G4UnionSolids`: 

<img width="650" height="650" alt="image" src="https://github.com/user-attachments/assets/4fd70eed-055c-496b-9522-01b0046ece9c" />

These boolean solids should work reasonably well with the quasiparticle and phonon physics in G4CMP, but...
1. ...deep nestings of G4UnionSolids to build comnplicated structures is not wise on performance grounds, and in my experience has occasionally made visualization choke as well. While functionally a triply- or quadruply-nested G4UnionSolid will work okay, consider using a G4MultiUnion if you're going to going to be attempting to link more than a couple basic `G4VSolid` objects (`G4Tubs`, `G4Box`, `G4Trd`, etc.). QP and phonon transport in `G4MultiUnion` objects has been lightly tested and anecdotal evidence points to it giving the correct film response, but this also needs further rigorous exploration, so proceed with caution and skepticism.
2. ...due to the current lack of generality of superconducting plane orientation currently available to the `qpDiffusion` process, one will find best results aligning their superconducting thin film's plane with _both_ the global XY plane _and_ the local XY coordinate systems of the constituent base solids (G4Box, G4Tubs, etc.). For most base solid geometries, this is what one might most naturally do anyway -- for example, a `G4Tubs`' local XY plane is the one described by ρ and φ, which is the plane that one would naturally keep coplanar with a film if one is attempting to build a curved structure into a thin film. However, solids like `G4Trd`, which one may use for a taper in a pad, set the local z direction to be in the direction of the taper, which requires a 90 degree rotation to embed them into the plane of a film. This may cause issues with QP propagation, but for now these issues can be temporarily circumvented by embedding these structures into a `G4UnionSolid` whose base element _does_ follow the coplanarity guidance given above. 

> [!TIP]
> It is possible to define complicated geometries with other construction techniques, which may be more efficient than what we do here. Challenge question: Can you think of a way to define portions of this geometry using the G4ExtrudedSolid class?



Though not explored here, tesselated geometries...?



### Boundary definitions

In addition to defining a proper third dimension for our films, we need to define additional boundaries so that phonons can reflect off of the vacuum-facing surfaces of the superconducting film. This is in contrast to old versions of this code, which only required interfaces of our superconducting (or vacuum) volumes with the silicon chip:

<img width="1138" height="361" alt="Screenshot 2026-06-07 at 3 34 11 AM" src="https://github.com/user-attachments/assets/bf2ad680-0c78-4b77-8f5a-ae1b2d530837" />






In G4CMP-V10, phonons continue to exist and travel in three dimensions. To make use of this transport within the superconducting film in our geometry, we have to properly define the third dimension of our film geometry. In this example, we define a film thickness of 


Now that transmission between volumes is possible, we need to define boundaries for  

For Phonons, which travel in 3 dimensions...

Phonons of all three polarizations travel in three dimensions, and now that transmission between volumes is possible, we need to ensure that 


For QPs, which...
- Mention the "keep all heights the same" here

Bilayers?


### Lattice Definitions



### Bilayers

This requires a refinement of how we define detector volumes. While in G4CMP-V9 code, we were able to get away with just defining the interface behavior of phonons at 

This requires a change in how we view volume definitions. In older, G4CMP-V9 code, the only thing that mattered in a simulation was surface coverage and deini





## Tutorial Example 2: Scanning Energy Depositions in a Planar Resonator














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




