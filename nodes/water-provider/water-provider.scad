use <threads.scad>;

thickness = 1;

// Tube
tubeInnerWidth = 4;
tubeConnectionInnerWidth = 2; //tubeInnerWidth - 2 * thickness;
tubeConnectionOuterWidth = tubeInnerWidth + 2;

// Axis ending
axisEndingHeight = 2;
axisEndingWidth = tubeConnectionInnerWidth + 2;

// Axis
axisDiameter = 4;

// Gear
gearPitch = 2;
gearToothTopSize = 0.5;
gearToothRootSize = 0.5;
gearToothHeight = 2;

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

module axisWaterBridge()
{
    axisWaterBridgeWidth = caseWidth - thickness * 2 - caseWidth / 4;
    axisWaterBridgeRadius = 10;
    axisWaterBridgeDepth = caseDepth - thickness * 2 - 0.5;
    axisWaterBridgeThickness = 1;

    translate([-axisWaterBridgeWidth / 2, 0, axisWaterBridgeRadius])
    {
        rotate(a=90, v = [0, 1, 0])
        {
            difference()
            {
                cylinder(h=axisWaterBridgeWidth, r=axisWaterBridgeRadius, center=false, $fn=100);
                cylinder(h=axisWaterBridgeWidth, r=axisWaterBridgeRadius - axisWaterBridgeThickness, center=false, $fn=100);
                translate([-axisWaterBridgeRadius * 2 + sqrt(axisWaterBridgeRadius*axisWaterBridgeRadius - axisWaterBridgeDepth*axisWaterBridgeDepth/4), -axisWaterBridgeRadius - 0.1, -0.1])
                {
                    cube([axisWaterBridgeRadius * 2, axisWaterBridgeRadius * 2 + 0.2, caseWidth + 0.2]);
                }
            }
        }
    }
}

module threadAxis(length)
{
    metric_thread(diameter=axisDiameter + 2, pitch=1, length=length);
}

module threadGear(length)
{
    metric_thread(diameter=axisDiameter + 2 + 0.1, pitch=1, length=length);
}

module axis()
{
    axisLength = caseTopHeight + caseMiddleHeight + caseBottomHeight - thickness * 2 - axisEndingHeight;
    axisThreadsOffset = caseTopHeight - thickness;
    axisThreadsLength = 2;
 
    // Small water bridge
    translate([0, 0, axisLength - axisEndingHeight / 2])
    {
        axisEnding();
        rotate(a=5, v=[0, 1, 0])
        {
            axisWaterBridge();
        }
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
        gear(6, gearAxisHeight);
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
        //translate([0, 0, thickness]) color("red") axis();
        //translate([0, 0, caseMiddleHeight + caseBottomHeight]) color("yellow") caseTop();
      //  translate([0, 0, caseBottomHeight]) gearAxis();
    }
    //translate([-caseWidth / 2 - 1, 0, -100 + caseMiddleHeight + caseBottomHeight + caseTopHeight - thickness]) cube([caseWidth + 2, 10, 100]);
    //translate([-caseWidth / 2 - 1, -10, -1]) cube([caseWidth + 2, 10, 100]);
}



difference()
{
    metric_thread(diameter=12, pitch=2, length=15, thread_size=3);
    translate([0, 0, -1]) cylinder(h=6, r=6 / 2, center=false, $fn=100);
    
    translate([0, 0, 4.9]) group()
    {
        difference()
        {
            cylinder(h=1.5, r=5 / 2, center=false, $fn=100);
            translate([-3, 0.5, -1]) cube([6, 6, 3]);
        }
        translate([-1.5, -1, -1]) cube([3, 3, 3]);
    }
}


