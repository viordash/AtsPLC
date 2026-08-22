import { Component, EventEmitter, Input, Output } from '@angular/core';

@Component({
	selector: 'app-confirm-dialog',
	standalone: true,
	templateUrl: './confirm-dialog.component.html',
	styleUrls: ['./confirm-dialog.component.scss']
})
export class ConfirmDialogComponent {
	@Input() message: string = 'Are you sure?';
	@Output() confirmed = new EventEmitter<boolean>();

	onOk(): void {
		this.confirmed.emit(true);
	}

	onCancel(): void {
		this.confirmed.emit(false);
	}
}
