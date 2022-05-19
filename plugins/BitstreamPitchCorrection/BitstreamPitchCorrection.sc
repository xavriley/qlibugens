BitstreamPitchCorrection : UGen {
	*ar { |input, low_f = 80.0, high_f = 1280.0, transpose = 0, target_note = -1, key = 0, scale = 0|
		/* TODO */
		^this.multiNew('audio', input, low_f, high_f, transpose, target_note, key, scale);
	}
	checkInputs {
		// Check the rate of the frequency argument
        if(inputs.at(1).rate != \scalar, {
            "Min frequency can only be set at scalar/initial rate".error
            });

        if(inputs.at(2).rate != \scalar, {
            "Max frequency can only be set at scalar/initial rate".error
            });

        /* TODO: add boundary checks on inputs */
        ^this.checkValidInputs;
	}
}
