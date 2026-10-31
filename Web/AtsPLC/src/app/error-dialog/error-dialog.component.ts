import { Component } from '@angular/core';
import { ErrorDialogService } from '../services/error-dialog.service';

@Component({
	selector: 'app-error-dialog',
	standalone: true,
	templateUrl: './error-dialog.component.html',
	styleUrls: ['./error-dialog.component.scss']
})
export class ErrorDialogComponent {
	constructor(public errorDialogService: ErrorDialogService) { }

	onOk(): void {
		this.errorDialogService.close();
	}
}
