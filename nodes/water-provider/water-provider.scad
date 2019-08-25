module hose() {
    difference()
    {
        cylinder(h=15, r=8, center=true, $fn=100);
        cylinder(h=20, r=5, center=true, $fn=100);
    }
}

hose();
