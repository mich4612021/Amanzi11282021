1D Sorption Isotherms
=====================

Overview and Capabilities tested
--------------------------------

This test example performs the simulation of sorption of aqueous components on a surface in a 1D flow domain, testing the following capabilities:

* 1D flow
* 1D advective transport 
* Geochemical reactions

	* Sorption isotherms (linear or Kd-approach, Freundlich and Langmuir)

For details on this test, see :ref:`about_isotherms`.

Background
----------

The simplest model to describe sorption of a component on a surface is to relate the sorbed concentration on the surface to the concentration in the solution, thereby neglecting any additional effects on the sorption processes. Different models are available for the form of the relationship between aqueous and sorbed concentrations: the linear isotherm (or Kd-approach), the Freundlich isotherm and the Langmuir isotherm. This test example simply considers three generic components, each of which sorbs to a surface according to one isotherm model.

Model
-----

Flow and transport 
~~~~~~~~~~~~~~~~~~

See the :doc:`../tracer_1d/amanzi_u-1d-tracer` example.

Primary species
~~~~~~~~~~~~~~~

Three generic component species are used: :math:`\ce{A}`, :math:`\ce{B}`, and :math:`\ce{C}`.

Isotherms
~~~~~~~~~

A different sorption of isotherm is consider for each component. With each isotherm model, the sorbed concentration (:math:`C_s`) is a function of the aqueous concentration of the component (:math:`C_{aq}`):

* The sorbed concentration of component :math:`\ce{A}` is calculated according to a **linear isotherm (or Kd-approach)**:

.. math::
  C_s = K_D \times C_{aq}
  :label: linear

* The sorbed concentration of component :math:`\ce{B}` is calculated according to a **Langmuir isotherm**:

.. math::
  C_s = \frac{ K \times C_{aq} \times b }{ 1 + K \times C_{aq} }
  :label: langmuir

* The sorbed concentration of component :math:`\ce{C}`  is calculated according to a **Freundlich isotherm**:

.. math::
  C_s = K_D \times C_{aq}^{1/n}
  :label: freundlich


Problem specifications
----------------------

Flow and transport 
~~~~~~~~~~~~~~~~~~

See the :doc:`../tracer_1d/amanzi_u-1d-tracer` example. In this example, a solution that contains the three components at the same concentration is injected at the left boundary. As they flow down gradient they sorb according the difference sorption isotherms. The simulation is run to 50 years.

Geochemistry 
~~~~~~~~~~~~

The initial concentration of the component in the domain is zero, while in the infiltrating solution their concentration is :math:`C_{aq}^A=C_{aq}^B=C_{aq}^C= 10^{-3} \text{mol/L}`.

The isotherm equation parameters are:

* Linear isotherm: :math:`K_D=10`.
* Langmuir isotherm: :math:`K=30` and :math:`b=0.1`.
* Freundlich isotherm: :math:`K_D=1.5` and :math:`n=0.8`.

Results and Comparison
----------------------

Simulation results
~~~~~~~~~~~~~~~~~~

The figure below shows both the aqueous and sorbed concentrations of :math:`\ce{A}, \ce{B}` and :math:`\ce{C}` along the flow direction at 50 years between :math:`x=30 m` and :math:`x=70 m`. Even though the infiltrating concentrations of the aqueous components are the same, the different models lead to different sorbed concentrations. Comparison to PFloTran results is hampered by the different numerical dispersion caused by the discretization schemes. However, the differences between isotherm model results show the same pattern as far as aqueous concentrations goes and the results in terms of sorbed concentrations are within close agreement.  

.. plot:: isotherms_1d.py

..   :align: left

.. _about_isotherms:

About
-----

* Benchmark simulator: PFlotran 
* Files:

  * Amanzi input file/s (native chemistry):  amanzi-u-1d-isotherms.xml
  * Amanzi input file/s (Alquimia chemistry): amanzi-1d-isotherms-alq.xml, 1d-isotherms.in, isotherms.dat 
  * Benchmark simulator input file: 1d-isotherms.in, isotherms.dat

* Location: testing/benchmarking/chemistry/isotherms_1d
* Author: B. Andre, G. Hammond
* Testing and Documentation: S. Molins
* Last tested on Nov 13, 2013
