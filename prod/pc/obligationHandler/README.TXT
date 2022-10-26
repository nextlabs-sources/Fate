#############################################################################
#
# Branding Change Support
#
#############################################################################

Testing - What to look for
--------------------------
The Policy Controller has two components that are affected by branding.  The
Windows Event Viewer shows the service by name, and it has been changed to be
brand neutral (to "Policy Controller").  Second, the Obligation Manager
contains both icons and text that reference a brand.  These are visible from
the tray icon and its menus.  These are the only affected areas of the
Obligation Manager.  Changing them is described below.

Summary of Change
-----------------
To support changes in branding (e.g. "Compliant Enterprise" to "Enterprise
DLP") there are a number of itmes that require modification.  There are
several source changes required to make branding changeable.  The resource
files must also define the additional branding such that the resource library
can be selected at install time.

A resource library will be built for each branding style and the installer
will select it.



Branding Change
---------------
After the above source changes are in place.  To affect brand change the
logos (icons), resource files, and project file needs to change.  Below are
the changes for adding 'Enterprise DLP' branding where only 'Compliant
Enterprise' branding was there previuosly.

Logos (icons):
DestinyNotify/small.ico					Compliant Enterprise
DestinyNotify/small_EDLP.ico				Enterprise DLP
DestinyNotify/DestinyNotify.ico				Compliant Enterprise
DestinyNotify/DestinyNotify_EDLP.ico			Enterprise DLP

Resource files:
DestinyNotify/DestinyNotify.rc				Resource file (CE)
DestinyNotify/DestinyNotify_EDLP.rc			Resource file (EDLP)
DestinyNotifyResource/DestinyNotifyResource.rc		Resource file (CE)
DestinyNotifyResource/DestinyNotifyResource_EDLP.rc	Resource file (EDLP)

Make files:
DestinyNotifyResource/DestinyNotifyResource.vcproj	Project file (CE)
DestinyNotifyResource/DestinyNotifyResource_EDLP.vcproj	Project file (EDLP)

Additional Branding
-------------------
When adding a new branding style the following must be done:

(1) Duplicate logos (icons) as is done with EDLP above.
(2) Duplicate resource files (.rc) as is done with EDLP above.
(3) Duplicate and modify DestinyNotifyResource_[brand].vcproj as is done
    with EDLP.  The target binary must be unique.
