import { Component, OnInit, OnDestroy } from '@angular/core';
import { Subscription } from 'rxjs';
import { SidebarComponent } from '../sidebar/sidebar.component';
import { CanvasAreaComponent } from '../canvas-area/canvas-area.component';
import { ConfirmDialogComponent } from '../confirm-dialog/confirm-dialog.component';
import { ErrorDialogComponent } from '../error-dialog/error-dialog.component';
import { ConfirmDialogService, ConfirmDialogRequest } from '../services/confirm-dialog.service';

@Component({
	selector: 'app-root',
	standalone: true,
	imports: [
		SidebarComponent,
		CanvasAreaComponent,
		ConfirmDialogComponent,
		ErrorDialogComponent
	],
	templateUrl: './main.component.html',
	styleUrls: ['./main.component.scss']
})
export class MainComponent implements OnInit, OnDestroy {
	showDialog: boolean = false;
	dialogMessage: string = '';
	private subscription: Subscription | null = null;

	constructor(private confirmDialogService: ConfirmDialogService) {}

	ngOnInit(): void {
		this.subscription = this.confirmDialogService.showRequest$.subscribe(
			(request: ConfirmDialogRequest) => {
				this.dialogMessage = request.message;
				this.showDialog = true;
			}
		);
	}

	ngOnDestroy(): void {
		this.subscription?.unsubscribe();
	}

	onDialogConfirmed(confirmed: boolean): void {
		this.showDialog = false;
		this.confirmDialogService.respond(confirmed);
	}
}
