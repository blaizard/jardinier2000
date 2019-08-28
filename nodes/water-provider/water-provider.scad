use <threads.scad>;

thickness = 1;

// Tube
tubeInnerWidth = 3;
tubeConnectionInnerWidth = 2; //tubeInnerWidth - 2 * thickness;
tubeConnectionOuterWidth = tubeInnerWidth + 1;

// Axis ending
axisEndingHeight = 2;
axisEndingWidth = tubeConnectionInnerWidth + 2;

// Axis
axisDiameter = 3;

// Gear
gearPitch = 2;
gearToothTopSize = 0.5;
gearToothRootSize = 0.5;
gearToothHeight = 1;

// Case
caseWidth = 20;
caseDepth = 8;

// Case top part
caseTopHeight = 10;
caseMiddleHeight = 5;
caseBottomHeight = 30;

module tubeConnection(length)
{
    connectionIncrement = 2;
    difference()
    {
        union()
        {
            cylinder(h=length, r=tubeInnerWidth / 2, center=false, $fn=100);
            for (i = [0:connectionIncrement:length-connectionIncrement])
            {
                translate([0, 0, i + length % connectionIncrement])
                {
                    cylinder(h=connectionIncrement, r2=tubeInnerWidth / 2, r1=tubeConnectionOuterWidth / 2, center=false, $fn=100);
                }
            }
        }
        translate([0, 0, -1])
        {
            cylinder(h=length + 2, r=tubeConnectionInnerWidth / 2, center=false, $fn=100);
        }
    }
}

/**
 * Design so that 1mm fits inside the innertube tube when full contact
 */
module axisEnding()
{
    cylinder(h=axisEndingHeight / 2, r1=0, r2=axisEndingWidth / 2, center=false, $fn=100);
    translate([0, 0, axisEndingHeight / 2])
    {
        cylinder(h=axisEndingHeight / 2, r1=axisEndingWidth / 2, r2=0, center=false, $fn=100);
    }
}

module threadAxis(length)
{
    metric_thread(diameter=axisDiameter + 1, pitch=0.5, length=length);
}

module threadGear(length)
{
    metric_thread(diameter=axisDiameter + 1 + 0.1, pitch=0.5, length=length);
}

module axis()
{
    axisLength = caseTopHeight + caseMiddleHeight + caseBottomHeight - thickness * 2 - axisEndingHeight;
    axisThreadsOffset = caseTopHeight - thickness;
    axisThreadsLength = 2;
 
    translate([0, 0, axisLength - axisEndingHeight / 2])
    {
        axisEnding();
    }
    cylinder(h=axisLength - axisThreadsOffset - axisThreadsLength, r=axisDiameter / 2, center=false, $fn=100);
    translate([0, 0, axisLength - axisThreadsOffset - axisThreadsLength])
    {
        threadAxis(axisThreadsLength);
    }
    translate([0, 0, axisLength - axisThreadsOffset])
    {
        cylinder(h=axisThreadsOffset, r=axisDiameter / 2, center=false, $fn=100);
    }
}

module gear(radius, height = 2)
{
    perimeter = 2 * PI * radius;
    nbTeeth = round(perimeter / gearPitch);
    actualPitch = perimeter / nbTeeth;
    echo("Actual pitch for gear of radius ", radius, "pitch=", actualPitch, "nb.teeth=", nbTeeth);

    difference()
    {
        cylinder(h=height, r=radius, center=false, $fn=100);
        union()
        {
            for (angle = [0:(actualPitch * 360 / perimeter):360])
            {
                rotate(a=angle, v=[0, 0, 1])
                {
                    translate([radius - gearToothHeight, 0, -1])
                    {
                        hull()
                        {
                            translate([0, 0, 0]) cylinder(r=gearToothRootSize/2, h=height + 2);
                            translate([gearToothHeight, actualPitch / 2 - gearToothTopSize / 2, 0]) cylinder(r=0.1, h=height + 2);
                            translate([gearToothHeight, -actualPitch / 2 + gearToothTopSize / 2, 0]) cylinder(r=0.1, h=height + 2);
                        }
                    }
                }
            }
        }
    }
}

module gearAxis()
{
    gearAxisHeight = caseMiddleHeight - 1;
    
    difference()
    {
        gear(4, gearAxisHeight);
        translate([0, 0, -1])
        {
            threadGear(gearAxisHeight + 2);
        }
    }
}

module caseTop()
{
    tubeConnectionLength = caseWidth / 2;
    tubeConnectionOffset = 5;

    difference()
    {
        union()
        {
            // Tube connection ending orthogonal
            translate([0 , 0, caseTopHeight + tubeConnectionOffset])
            {
                rotate(a=90, v=[0, 1, 0])
                {
                    tubeConnection(tubeConnectionLength);
                }
            }
            // Tube connection link
            translate([0 , 0, caseTopHeight - 0.1])
            {
                cylinder(h=tubeConnectionOffset + tubeConnectionOuterWidth / 2 + 0.1, r=tubeConnectionOuterWidth / 2, center=false, $fn=100);
            }
            // Box
            translate([-caseWidth / 2, -caseDepth / 2, 0])
            {
                difference()
                {
                    cube([caseWidth, caseDepth, caseTopHeight]);
                    translate([thickness, thickness, thickness])
                    {
                        cube([caseWidth - thickness * 2, caseDepth - thickness * 2, caseTopHeight - thickness * 2]);
                    }
                }
            }
        }
        // Hole tube connection ending orthogonal
        translate([0 , 0, caseTopHeight + tubeConnectionOffset])
        {
            rotate(a=90, v=[0, 1, 0])
            {
                cylinder(h=tubeConnectionLength, r=tubeConnectionInnerWidth / 2, center=false, $fn=100);
            }
        }
        // Hole tube connection link
        translate([0 , 0, caseTopHeight - thickness - 0.1])
        {
            cylinder(h=tubeConnectionOffset + tubeConnectionInnerWidth / 2 + thickness + 0.1, r=tubeConnectionInnerWidth / 2, center=false, $fn=100);
        }
        // Hole axis
        translate([0 , 0, - 0.1])
        {
            cylinder(h=thickness + 0.2, r=axisDiameter / 2 + 0.5, center=false, $fn=100);
        }
        
    }
}

difference()
{
    union()
    {
        //tubeConnection(5);
        //axisEnding();
        translate([0, 0, thickness]) color("red") axis();
        translate([0, 0, caseMiddleHeight + caseBottomHeight]) color("yellow") caseTop();
        translate([0, 0, caseBottomHeight]) gearAxis(5);
    }
    translate([-caseWidth / 2 - 1, 0, -1]) cube([caseWidth + 2, 10, 100]);
}